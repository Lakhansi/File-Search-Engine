// src/Index.h
#ifndef INDEX_H
#define INDEX_H

#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm> // ADD for std::sort
#include "Trie.h"

struct FileMetadata {
    std::filesystem::path path;
    std::string filename;
    uintmax_t size;
    std::filesystem::file_time_type last_modified;
    std::string extension;
    std::string content;
};

// NEW: Ranking criteria enum
enum class SortBy {
    NAME,
    SIZE_ASC,
    SIZE_DESC,
    DATE_ASC,
    DATE_DESC,
    RELEVANCE
};

class Index {
public:
    Index();
    ~Index();

    void addFile(const FileMetadata& data);
    const std::vector<FileMetadata>& getAllFiles() const;

    std::vector<FileMetadata> searchByFilename(const std::string& filename, SortBy sort = SortBy::NAME) const;
    std::vector<FileMetadata> searchByPrefix(const std::string& prefix, SortBy sort = SortBy::NAME) const;
    std::vector<FileMetadata> searchByExtension(const std::string& extension, SortBy sort = SortBy::NAME) const;
    std::vector<FileMetadata> searchByContent(const std::string& query, SortBy sort = SortBy::RELEVANCE) const;

    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    void clear();

private:
    std::vector<FileMetadata> m_files;
    std::unordered_map<std::string, std::vector<size_t>> m_filename_map;
    std::unordered_map<std::string, std::vector<size_t>> m_extension_map;
    std::unordered_map<std::string, std::unordered_set<size_t>> m_inverted_index;
    Trie m_filename_trie;

    void indexFileContent(const std::string& content, size_t file_index);
    std::vector<std::string> extractWords(const std::string& text) const;
    std::string toLowerCase(const std::string& str) const;
    bool isTextFile(const std::string& extension) const;

    // NEW: Ranking helper functions
    std::vector<FileMetadata> sortResults(std::vector<FileMetadata> results, SortBy criteria) const;
    int calculateRelevance(const FileMetadata& file, const std::vector<std::string>& query_words) const;

    void writeString(std::ofstream& out, const std::string& str) const;
    std::string readString(std::ifstream& in) const;
    void writeMap(std::ofstream& out, const std::unordered_map<std::string, std::vector<size_t>>& map) const;
    void readMap(std::ifstream& in, std::unordered_map<std::string, std::vector<size_t>>& map);
    void writeInvertedIndex(std::ofstream& out) const;
    void readInvertedIndex(std::ifstream& in);
};

#endif // INDEX_H
