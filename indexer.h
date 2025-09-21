// Indexer.h
#ifndef INDEXER_H
#define INDEXER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include "Index.h"

class Indexer {
public:
    Indexer(Index& index);
    ~Indexer();

    void setRootPath(const std::string& path);
    void run();
    void stop(); 
    void stopImmediately() { m_stopRequested = true; }
    
    bool scanDirectorySafe(const std::filesystem::path& path);

private:
    Index& m_index;
    std::string m_rootPath;

    
    std::vector<std::thread> m_workerThreads;
    std::queue<std::filesystem::path> m_fileQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    std::atomic<bool> m_stopRequested{false};
    std::atomic<int> m_filesProcessed{0};
    int m_totalFiles{0};

    void workerThread(); 
    void processFile(const std::filesystem::path& filePath); 
};

#endif 

