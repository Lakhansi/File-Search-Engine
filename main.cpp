// main.cpp
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <windows.h>
#include "Index.h"
#include "Indexer.h"

bool isAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(adminGroup);
    }

    return isAdmin == TRUE;
}

void runAsAdmin() {
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    ShellExecute(NULL, "runas", path, NULL, NULL, SW_SHOWNORMAL);
    exit(0);
}


std::string getRootPathFromUser() {
    std::string path;
    std::cout << "\n📁 Enter the directory path to scan: ";
    std::getline(std::cin, path);

    if (path.empty()) {
        return ".\\";
    }

  
    std::replace(path.begin(), path.end(), '/', '\\');


    if (path.back() != '\\' && path.find('.') == std::string::npos) {
        path += '\\';
    }

    return path;
}

bool shouldUseCache() {
    char choice;
    std::cout << "💾 Use cached index if available? (y/n): ";
    std::cin >> choice;
    std::cin.ignore(); 
    return (choice == 'y' || choice == 'Y');
}

int main() {
    std::cout << "Starting File Search App (Now with Caching!)..." << std::endl;

    Index index;
    const std::string cacheFile = "./index_cache.bin";

    // Get user input for directory path
    std::string rootPath = getRootPathFromUser();

    if ((rootPath == "C:\\" || rootPath.find("C:\\\\") != std::string::npos) && !isAdmin()) {
        std::cout << "\n⚠️  WARNING: Scanning C:\\ requires Administrator privileges!" << std::endl;
        std::cout << "   Some system directories will be inaccessible." << std::endl;
        std::cout << "   Continue anyway? (y/n): ";

        char choice;
        std::cin >> choice;
        std::cin.ignore();

        if (choice != 'y' && choice != 'Y') {
            std::cout << "Operation cancelled." << std::endl;
            return 0;
        }
    }

    bool useCache = shouldUseCache();

    if (useCache) {
        std::cout << "Attempting to load index from cache..." << std::endl;
        if (index.loadFromFile(cacheFile)) {
            std::cout << "✅ Successfully loaded index from cache!" << std::endl;
            std::cout << "📊 Total files in index: " << index.getAllFiles().size() << std::endl;
        } else {
            std::cout << "❌ No cache found or cache invalid. Indexing files..." << std::endl;
            useCache = false;
        }
    }

    if (!useCache) {
        std::cout << "❌ No cache found or cache invalid. Indexing files..." << std::endl;

        Indexer indexer(index);
        indexer.setRootPath(rootPath);

        auto start = std::chrono::high_resolution_clock::now();
        indexer.run();
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "⏱️  Indexing completed in " << duration.count() << " milliseconds" << std::endl;

        std::cout << "💾 Saving index to cache..." << std::endl;
        if (index.saveToFile(cacheFile)) {
            std::cout << "✅ Cache saved successfully!" << std::endl;
        } else {
            std::cout << "❌ Failed to save cache." << std::endl;
        }
    }

    std::string searchTerm;
    std::cout << "\n🔍 === Interactive Search Mode ===" << std::endl;

    while (true) {
        std::cout << "\nEnter search term (or 'quit' to exit): ";
        std::getline(std::cin, searchTerm);

        if (searchTerm == "quit" || searchTerm == "exit") {
            break;
        }

        if (searchTerm.empty()) {
            continue;
        }

        std::cout << "\n📝 Files containing '" << searchTerm << "':" << std::endl;
        auto results = index.searchByContent(searchTerm);

        if (results.empty()) {
            std::cout << "No files found containing '" << searchTerm << "'" << std::endl;
        } else {
            for (const auto& file : results) {
                std::cout << "• " << file.filename << " | " << file.size << " bytes" << std::endl;
                std::cout << "  Path: " << file.path << std::endl;
            }
            std::cout << "Found " << results.size() << " file(s)" << std::endl;
        }

        if (searchTerm.size() <= 5 && searchTerm.find('.') != std::string::npos) {
            std::cout << "\n📁 Files with extension '" << searchTerm << "':" << std::endl;
            auto extResults = index.searchByExtension(searchTerm);

            if (extResults.empty()) {
                std::cout << "No files found with extension '" << searchTerm << "'" << std::endl;
            } else {
                for (const auto& file : extResults) {
                    std::cout << "• " << file.filename << " | " << file.size << " bytes" << std::endl;
                }
                std::cout << "Found " << extResults.size() << " file(s)" << std::endl;
            }
        }
    }

    std::cout << "\n💡 Tips for next time:" << std::endl;
    std::cout << "💡 - Delete 'index_cache.bin' to force re-indexing" << std::endl;
    std::cout << "💡 - Run as Administrator to scan system directories" << std::endl;
    std::cout << "💡 - Try scanning specific folders instead of entire drives" << std::endl;

    return 0;
}

