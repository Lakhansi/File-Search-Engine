// Index.cpp
#include "Index.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cctype>
#include <iostream>
#include "utils.h"

namespace fs = std::filesystem;

Index::Index() {
  
}

Index::~Index() {
  
}


std::string Index::toLowerCase(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::vector<std::string> Index::extractWords(const std::string& text) const {
    std::vector<std::string> words;
    std::stringstream ss(text);
    std::string word;

    while (ss >> word) {

        word.erase(std::remove_if(word.begin(), word.end(),
                                  [](unsigned char c) { return std::ispunct(c); }), word.end());
        if (!word.empty()) {
            words.push_back(toLowerCase(word));
        }
    }

    return words;
}

void Index::writeString(std::ofstream& out, const std::string& str) const {
    size_t length = str.size();
    out.write(reinterpret_cast<const char*>(&length), sizeof(length));
    out.write(str.c_str(), length);
}

bool Index::isTextFile(const std::string& extension) const {
    static const std::unordered_set<std::string> text_extensions = {
        "txt", "cpp", "c", "h", "hpp", "py", "java", "js", "html", "css",
        "xml", "json", "csv", "md", "log", "conf", "config", "ini", "bat", "sh"
    };
    std::string ext_lower = toLowerCase(extension);
    return text_extensions.find(ext_lower) != text_extensions.end();
}

void Index::indexFileContent(const std::string& content, size_t file_index) {
    if (content.empty()) return;

    std::vector<std::string> words = extractWords(content);
    for (const auto& word : words) {
        m_inverted_index[word].insert(file_index);
    }
}

void Index::addFile(const FileMetadata& data) {
    m_files.push_back(data);
    size_t current_index = m_files.size() - 1;

    std::string key = data.filename;
    auto it = m_filename_map.find(key);
    if (it != m_filename_map.end()) {
        it->second.push_back(current_index);
    } else {
        m_filename_map[key] = {current_index};
    }

    m_filename_trie.insert(data.filename, current_index);

    std::string extension = data.extension;
    if (!extension.empty()) {
        std::string ext_lower = toLowerCase(extension);
        auto ext_it = m_extension_map.find(ext_lower);
        if (ext_it != m_extension_map.end()) {
            ext_it->second.push_back(current_index);
        } else {
            m_extension_map[ext_lower] = {current_index};
        }

        if (isTextFile(ext_lower)) {
            indexFileContent(data.content, current_index);
        }
    }
}

const std::vector<FileMetadata>& Index::getAllFiles() const {
    return m_files;
}

std::vector<FileMetadata> Index::sortResults(std::vector<FileMetadata> results, SortBy criteria) const {
    switch (criteria) {
    case SortBy::SIZE_ASC:
        std::sort(results.begin(), results.end(),
                  [](const FileMetadata& a, const FileMetadata& b) { return a.size < b.size; });
        break;

    case SortBy::SIZE_DESC:
        std::sort(results.begin(), results.end(),
                  [](const FileMetadata& a, const FileMetadata& b) { return a.size > b.size; });
        break;

    case SortBy::DATE_ASC:
        std::sort(results.begin(), results.end(),
                  [](const FileMetadata& a, const FileMetadata& b) { return a.last_modified < b.last_modified; });
        break;

    case SortBy::DATE_DESC:
        std::sort(results.begin(), results.end(),
                  [](const FileMetadata& a, const FileMetadata& b) { return a.last_modified > b.last_modified; });
        break;

    case SortBy::RELEVANCE:

        break;

    case SortBy::NAME:
    default:
        std::sort(results.begin(), results.end(),
                  [](const FileMetadata& a, const FileMetadata& b) { return a.filename < b.filename; });
        break;
    }
    return results;
}

int Index::calculateRelevance(const FileMetadata& file, const std::vector<std::string>& query_words) const {
    if (file.content.empty()) return 0;

    int score = 0;
    std::string content_lower = toLowerCase(file.content);

    for (const auto& word : query_words) {
        size_t pos = 0;
        while ((pos = content_lower.find(word, pos)) != std::string::npos) {
            score += 10;
            pos += word.length();

            if (pos < content_lower.length()) {
                char next_char = content_lower[pos];
                if (!std::isalpha(next_char)) {
                    score += 5;
                }
            }
        }
    }

    return score;
}

std::vector<FileMetadata> Index::searchByFilename(const std::string& filename, SortBy sort) const {
    std::vector<FileMetadata> results;
    auto it = m_filename_map.find(filename);
    if (it != m_filename_map.end()) {
        for (size_t index : it->second) {
            results.push_back(m_files[index]);
        }
    }
    return sortResults(results, sort);
}

std::vector<FileMetadata> Index::searchByPrefix(const std::string& prefix, SortBy sort) const {
    std::vector<FileMetadata> results;
    std::vector<size_t> indices = m_filename_trie.searchPrefix(prefix);
    for (size_t index : indices) {
        results.push_back(m_files[index]);
    }
    return sortResults(results, sort);
}

std::vector<FileMetadata> Index::searchByExtension(const std::string& extension, SortBy sort) const {
    std::vector<FileMetadata> results;

    std::string ext_lower = extension;
    std::transform(ext_lower.begin(), ext_lower.end(), ext_lower.begin(), ::tolower);

    auto it = m_extension_map.find(ext_lower);
    if (it != m_extension_map.end()) {
        for (size_t index : it->second) {
            results.push_back(m_files[index]);
        }
    }

    return sortResults(results, sort);
}

std::vector<FileMetadata> Index::searchByContent(const std::string& query, SortBy sort) const {
    std::vector<FileMetadata> results;
    std::unordered_set<size_t> matching_indices;

    std::vector<std::string> query_words = extractWords(query);
    if (query_words.empty()) {
        return results;
    }

    // For the first word, get all files that contain it
    auto it = m_inverted_index.find(query_words[0]);
    if (it != m_inverted_index.end()) {
        matching_indices = it->second;
    }

    for (size_t i = 1; i < query_words.size(); i++) {
        auto word_it = m_inverted_index.find(query_words[i]);
        if (word_it != m_inverted_index.end()) {
            std::unordered_set<size_t> temp;
            for (size_t index : matching_indices) {
                if (word_it->second.find(index) != word_it->second.end()) {
                    temp.insert(index);
                }
            }
            matching_indices = std::move(temp);
        } else {
            matching_indices.clear();
            break;
        }
    }

    for (size_t index : matching_indices) {
        results.push_back(m_files[index]);
    }

    if (sort == SortBy::RELEVANCE && !results.empty()) {
        std::vector<std::pair<int, FileMetadata>> scored_results;
        for (auto& file : results) {
            int score = calculateRelevance(file, query_words);
            scored_results.emplace_back(score, file);
        }

        std::sort(scored_results.begin(), scored_results.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });

        results.clear();
        for (auto& scored_file : scored_results) {
            results.push_back(scored_file.second);
        }
    } else {
        results = sortResults(results, sort);
    }

    return results;
}

std::string Index::readString(std::ifstream& in) const {
    size_t length;
    in.read(reinterpret_cast<char*>(&length), sizeof(length));
    std::string str(length, '\0');
    in.read(&str[0], length);
    return str;
}

void Index::writeMap(std::ofstream& out, const std::unordered_map<std::string, std::vector<size_t>>& map) const {
    size_t mapSize = map.size();
    out.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

    for (const auto& pair : map) {
        writeString(out, pair.first); // key
        size_t vecSize = pair.second.size();
        out.write(reinterpret_cast<const char*>(&vecSize), sizeof(vecSize));
        for (size_t value : pair.second) {
            out.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
    }
}

void Index::readMap(std::ifstream& in, std::unordered_map<std::string, std::vector<size_t>>& map) {
    size_t mapSize;
    in.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    for (size_t i = 0; i < mapSize; ++i) {
        std::string key = readString(in);
        size_t vecSize;
        in.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));

        std::vector<size_t> values(vecSize);
        for (size_t j = 0; j < vecSize; ++j) {
            size_t value;
            in.read(reinterpret_cast<char*>(&value), sizeof(value));
            values[j] = value;
        }

        map[key] = values;
    }
}

void Index::writeInvertedIndex(std::ofstream& out) const {
    size_t mapSize = m_inverted_index.size();
    out.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

    for (const auto& pair : m_inverted_index) {
        writeString(out, pair.first); 
        size_t setSize = pair.second.size();
        out.write(reinterpret_cast<const char*>(&setSize), sizeof(setSize));
        for (size_t value : pair.second) {
            out.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
    }
}


void Index::readInvertedIndex(std::ifstream& in) {
    size_t mapSize;
    in.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    for (size_t i = 0; i < mapSize; ++i) {
        std::string key = readString(in);
        size_t setSize;
        in.read(reinterpret_cast<char*>(&setSize), sizeof(setSize));

        std::unordered_set<size_t> values;
        for (size_t j = 0; j < setSize; ++j) {
            size_t value;
            in.read(reinterpret_cast<char*>(&value), sizeof(value));
            values.insert(value);
        }

        m_inverted_index[key] = values;
    }
}


bool Index::saveToFile(const std::string& filename) const {
    try {
        std::ofstream out(filename, std::ios::binary);
        if (!out) return false;


        const int version = 1;
        out.write(reinterpret_cast<const char*>(&version), sizeof(version));


        size_t fileCount = m_files.size();
        out.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));
        for (const auto& file : m_files) {
            writeString(out, file.path.string());
            writeString(out, file.filename);
            out.write(reinterpret_cast<const char*>(&file.size), sizeof(file.size));
            out.write(reinterpret_cast<const char*>(&file.last_modified), sizeof(file.last_modified));
            writeString(out, file.extension);
            writeString(out, file.content);
        }

      
        writeMap(out, m_filename_map);
        writeMap(out, m_extension_map);
        writeInvertedIndex(out);

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving index: " << e.what() << std::endl;
        return false;
    }
}

bool Index::loadFromFile(const std::string& filename) {
    try {
        std::ifstream in(filename, std::ios::binary);
        if (!in) return false;

        int version;
        in.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != 1) {
            std::cerr << "Unsupported cache version: " << version << std::endl;
            return false;
        }

        clear();

        size_t fileCount;
        in.read(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));
        m_files.resize(fileCount);

        for (size_t i = 0; i < fileCount; ++i) {
            m_files[i].path = readString(in);
            m_files[i].filename = readString(in);
            in.read(reinterpret_cast<char*>(&m_files[i].size), sizeof(m_files[i].size));
            in.read(reinterpret_cast<char*>(&m_files[i].last_modified), sizeof(m_files[i].last_modified));
            m_files[i].extension = readString(in);
            m_files[i].content = readString(in);
        }

      
        readMap(in, m_filename_map);
        readMap(in, m_extension_map);
        readInvertedIndex(in);

        m_filename_trie.clear();
        for (const auto& entry : m_filename_map) {
            for (size_t index : entry.second) {
                m_filename_trie.insert(entry.first, index);
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading index: " << e.what() << std::endl;
        return false;
    }
}


void Index::clear() {
    m_files.clear();
    m_filename_map.clear();
    m_extension_map.clear();
    m_inverted_index.clear();
    m_filename_trie.clear();
}

