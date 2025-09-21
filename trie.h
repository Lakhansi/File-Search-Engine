// src/Trie.h
#ifndef TRIE_H
#define TRIE_H

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

struct TrieNode {
    std::unordered_map<char, TrieNode*> children;
    bool isEndOfWord = false;
    std::vector<size_t> fileIndices;

    // NEW: Serialization support
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & children & isEndOfWord & fileIndices;
    }
};

class Trie {
public:
    Trie();
    ~Trie();

    void insert(const std::string& word, size_t fileIndex);
    std::vector<size_t> searchPrefix(const std::string& prefix) const;
    void clear(); // NEW: Clear the trie

    // NEW: Serialization support
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & root;
    }

private:
    TrieNode* root;

    void clearRecursive(TrieNode* node);
    void collectIndicesRecursive(TrieNode* node, std::vector<size_t>& results) const;
};

#endif // TRIE_H
