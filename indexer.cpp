// src/Indexer.cpp
#include "Indexer.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "utils.h"

namespace fs = std::filesystem;

Indexer::Indexer(Index& index) : m_index(index) {}

Indexer::~Indexer() {
    stop();
}

void Indexer::setRootPath(const std::string& path) {
    m_rootPath = path;
}

void Indexer::stop() {
    m_stopRequested = true;
    m_queueCV.notify_all();

    for (auto& thread : m_workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    m_workerThreads.clear();
}

// NEW: Worker thread function
void Indexer::workerThread() {
    while (!m_stopRequested) {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_queueCV.wait(lock, [this]() {
            return !m_fileQueue.empty() || m_stopRequested;
        });

        if (m_stopRequested) break;

        if (!m_fileQueue.empty()) {
            fs::path filePath = m_fileQueue.front();
            m_fileQueue.pop();
            lock.unlock();

            processFile(filePath);

            // Update progress
            int processed = ++m_filesProcessed;
            if (processed % 10 == 0) { // Update every 10 files
                std::cout << "Processed " << processed << " of " << m_totalFiles
                          << " files (" << (processed * 100 / m_totalFiles) << "%)" << std::endl;
            }
        }
    }
}

// NEW: Process individual file
void Indexer::processFile(const fs::path& filePath) {
    try {
        FileMetadata data;
        data.path = filePath;
        data.filename = filePath.filename().string();
        data.size = fs::file_size(filePath);
        data.last_modified = fs::last_write_time(filePath);
        data.extension = filePath.extension().string();

        if (!data.extension.empty() && data.extension[0] == '.') {
            data.extension = data.extension.substr(1);
        }

        std::string ext_lower = data.extension;
        std::transform(ext_lower.begin(), ext_lower.end(), ext_lower.begin(), ::tolower);

        static const std::unordered_set<std::string> text_extensions = {
            "txt", "cpp", "c", "h", "hpp", "py", "java", "js", "html", "css",
            "xml", "json", "csv", "md", "log", "conf", "config", "ini", "bat", "sh"
        };

        if (text_extensions.find(ext_lower) != text_extensions.end() && data.size < 1048576) {
            data.content = readFileContent(filePath);
        } else {
            data.content = "";
        }

        // Use a lock to safely add to the index
        std::lock_guard<std::mutex> indexLock(m_queueMutex);
        m_index.addFile(data);

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error processing file: " << filePath << " - " << e.what() << std::endl;
    }
}

void Indexer::run() {
    if (m_rootPath.empty()) {
        std::cerr << "Error: Root path not set!" << std::endl;
        return;
    }

    std::cout << "Attempting to index directory: " << m_rootPath << std::endl;

    // Reset state
    m_stopRequested = false;
    m_filesProcessed = 0;
    m_totalFiles = 0;

    try {
        // First, count all files and build the queue
        std::cout << "Scanning directory structure..." << std::endl;
        for (const auto& entry : fs::recursive_directory_iterator(m_rootPath)) {
            if (entry.is_regular_file()) {
                m_totalFiles++;
                m_fileQueue.push(entry.path());
            }
        }

        std::cout << "Found " << m_totalFiles << " files. ";

        // NEW: Optimize thread count based on workload size
        unsigned int numThreads;
        if (m_totalFiles < 20) {
            // For small directories, use single thread (avoid overhead)
            numThreads = 1;
            std::cout << "Using single-threaded mode (small directory)" << std::endl;
        } else {
            // For larger directories, use multiple threads
            numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0) numThreads = 4;
            // Don't use more threads than files
            numThreads = std::min(numThreads, static_cast<unsigned int>(m_totalFiles));
            std::cout << "Using " << numThreads << " threads for indexing" << std::endl;
        }

        if (m_totalFiles == 0) {
            std::cout << "No files to index." << std::endl;
            return;
        }

        // NEW: For very small workloads, process directly without threads
        if (numThreads == 1) {
            std::cout << "Processing files..." << std::endl;
            while (!m_fileQueue.empty() && !m_stopRequested) {
                fs::path filePath = m_fileQueue.front();
                m_fileQueue.pop();
                processFile(filePath);
                m_filesProcessed++;
            }
        } else {
            // Use multithreading for larger workloads
            std::cout << "Starting multithreaded indexing..." << std::endl;

            // Create worker threads
            for (unsigned int i = 0; i < numThreads; ++i) {
                m_workerThreads.emplace_back(&Indexer::workerThread, this);
            }

            // Wait for all threads to complete
            for (auto& thread : m_workerThreads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }

            m_workerThreads.clear();
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        stop();
    }

    std::cout << "Indexing complete! Processed " << m_filesProcessed << " files." << std::endl;
}
