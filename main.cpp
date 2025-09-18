// src/main.cpp
#include <iostream>
#include <fstream>
#include <chrono>
#include "Index.h"
#include "Indexer.h"

int main() {
    std::cout << "Starting File Search App (Now with Caching!)..." << std::endl;

    Index index;
    const std::string cacheFile = "./index_cache.bin";
    
    // Try to load from cache first
    std::cout << "Attempting to load index from cache..." << std::endl;
    if (index.loadFromFile(cacheFile)) {
        std::cout << "✅ Successfully loaded index from cache!" << std::endl;
        std::cout << "📊 Total files in index: " << index.getAllFiles().size() << std::endl;
    } else {
        std::cout << "❌ No cache found or cache invalid. Indexing files..." << std::endl;
        
        Indexer indexer(index);
        indexer.setRootPath("./test_folder");
        
        auto start = std::chrono::high_resolution_clock::now();
        indexer.run();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "⏱️  Indexing completed in " << duration.count() << " milliseconds" << std::endl;
        
        // Save to cache for next time
        std::cout << "💾 Saving index to cache..." << std::endl;
        if (index.saveToFile(cacheFile)) {
            std::cout << "✅ Cache saved successfully!" << std::endl;
        } else {
            std::cout << "❌ Failed to save cache." << std::endl;
        }
    }
    
    // Test that everything works
    std::cout << "\n=== Testing Search After Cache Load ===" << std::endl;
    
    std::cout << "\n📝 Text files found:" << std::endl;
    auto results = index.searchByExtension("txt");
    for (const auto& file : results) {
        std::cout << "• " << file.filename << " | " << file.size << " bytes" << std::endl;
    }
    
    std::cout << "\n🔍 Content search for 'test':" << std::endl;
    results = index.searchByContent("test");
    for (const auto& file : results) {
        std::cout << "• " << file.filename << std::endl;
    }
    
    std::cout << "\n🔍 Content search for 'content':" << std::endl;
    results = index.searchByContent("content");
    for (const auto& file : results) {
        std::cout << "• " << file.filename << std::endl;
    }
    
    // Test cache invalidation by checking if files changed
    std::cout << "\n💡 To test cache, run the program again - it should load from cache instantly!" << std::endl;
    std::cout << "💡 Delete 'index_cache.bin' to force re-indexing" << std::endl;

    return 0;
}
