//Indexer.cpp
#include "Indexer.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <vector>
#include <windows.h>
#include "utils.h"

namespace fs = std::filesystem;

Indexer::Indexer(Index& index) : m_index(index), m_stopRequested(false) {}

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

void Indexer::workerThread() {
    while (!m_stopRequested) {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        if (m_queueCV.wait_for(lock, std::chrono::milliseconds(100), [this]() {
                return !m_fileQueue.empty() || m_stopRequested;
            })) {
            if (m_stopRequested) break;

            if (!m_fileQueue.empty()) {
                fs::path filePath = m_fileQueue.front();
                m_fileQueue.pop();
                lock.unlock();

                processFile(filePath);

                int processed = ++m_filesProcessed;
                if (processed % 50 == 0) {
                    std::cout << "Processed " << processed << " of " << m_totalFiles
                              << " files (" << (processed * 100 / m_totalFiles) << "%)" << std::endl;
                }
            }
        }
    }
}

void Indexer::processFile(const fs::path& filePath) {
    try {
        FileMetadata data;
        data.path = filePath;
        data.filename = filePath.filename().string();

        std::string ext = filePath.extension().string();
        if (!ext.empty() && ext[0] == '.') {
            ext = ext.substr(1);
        }

        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        data.extension = ext;

        try {
            data.size = fs::file_size(filePath);
            data.last_modified = fs::last_write_time(filePath);
        } catch (...) {
            return;
        }

        static const std::unordered_set<std::string> text_extensions = {
            "txt", "cpp", "c", "h", "hpp", "py", "java", "js", "html", "css",
            "xml", "json", "csv", "md", "log", "conf", "config", "ini", "bat", "sh"
        };

        bool is_text_file = (text_extensions.find(ext) != text_extensions.end());

        if (is_text_file && data.size > 0 && data.size < 1048576) {
            data.content = readFileContent(filePath);
        } else {
            data.content = "";
        }

        std::lock_guard<std::mutex> indexLock(m_queueMutex);
        m_index.addFile(data);

    } catch (...) {
    }
}


bool Indexer::scanDirectorySafe(const fs::path& path) {
    auto options = fs::directory_options::skip_permission_denied;
    int fileCount = 0;

    try {
        for (const auto& entry : fs::directory_iterator(path, options)) {
            if (m_stopRequested) return false;

            try {
                if (entry.is_regular_file()) {
                    m_totalFiles++;
                    m_fileQueue.push(entry.path());
                    fileCount++;
                }
                else if (entry.is_directory()) {
              
                    scanDirectorySafe(entry.path());
                }
            } catch (const fs::filesystem_error& e) {
 
                continue;
            } catch (...) {
                continue;
            }
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Skipping inaccessible directory: " << path << " - " << e.what() << std::endl;
        return false;
    }
}

void Indexer::run() {
    if (m_rootPath.empty()) {
        std::cerr << "Error: Root path not set!" << std::endl;
        return;
    }

    std::cout << "🚀 Scanning: " << m_rootPath << std::endl;

    m_stopRequested = false;
    m_filesProcessed = 0;
    m_totalFiles = 0;

    try {
        if (!fs::exists(m_rootPath)) {
            std::cerr << "Error: Directory does not exist!" << std::endl;
            return;
        }

        if (!fs::is_directory(m_rootPath)) {
            std::cerr << "Error: Path is not a directory!" << std::endl;
            return;
        }

        std::cout << "📁 Building file list (safe mode)..." << std::endl;

        bool success = scanDirectorySafe(m_rootPath);

        if (m_stopRequested) {
            std::cout << "⏹️  Scanning stopped" << std::endl;
            return;
        }

        if (m_totalFiles == 0) {
            if (!success) {
                std::cout << "❌ Could not access any files in the directory." << std::endl;
                std::cout << "💡 Try running as Administrator or choose a different directory." << std::endl;
            } else {
                std::cout << "ℹ️  No files found in directory." << std::endl;
            }
            return;
        }

        std::cout << "✅ Found " << m_totalFiles << " accessible files" << std::endl;

        std::cout << "🧵 Using single thread" << std::endl;
        std::cout << "⚡ Processing files..." << std::endl;

        auto startTime = std::chrono::steady_clock::now();

        while (!m_fileQueue.empty() && !m_stopRequested) {
            fs::path filePath = m_fileQueue.front();
            m_fileQueue.pop();
            processFile(filePath);
            m_filesProcessed++;

            if (m_filesProcessed % 10 == 0) {
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
                std::cout << "Processed " << m_filesProcessed << "/" << m_totalFiles
                          << " (" << (m_filesProcessed * 100 / m_totalFiles) << "%)"
                          << " in " << elapsed << "ms" << std::endl;
            }
        }

        auto endTime = std::chrono::steady_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        std::cout << "✅ Complete! Processed " << m_filesProcessed << " files in " << totalTime << "ms" << std::endl;
        if (m_filesProcessed > 0) {
            std::cout << "⏱️  Average: " << (totalTime / m_filesProcessed) << "ms per file" << std::endl;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

