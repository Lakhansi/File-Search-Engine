// rie.cpp
#include "Trie.h"
#include <iostream>
#include <functional> 

Trie::Trie() {
    root = new TrieNode();
}

Trie::~Trie() {
    clearRecursive(root);
}

void Trie::insert(const std::string& word, size_t fileIndex) {
    TrieNode* current = root;

    for (char c : word) {
        if (current->children.find(c) == current->children.end()) {
            current->children[c] = new TrieNode();
        }
        current = current->children[c];
    }

    current->isEndOfWord = true;
    current->fileIndices.push_back(fileIndex);
}

std::vector<size_t> Trie::searchPrefix(const std::string& prefix) const {
    std::vector<size_t> results;
    TrieNode* current = root;


    for (char c : prefix) {
        if (current->children.find(c) == current->children.end()) {
            return results; 
        }
        current = current->children[c];
    }


    std::function<void(TrieNode*)> collectIndices = [&](TrieNode* node) {
        if (!node) return;

        results.insert(results.end(), node->fileIndices.begin(), node->fileIndices.end());


        for (auto& child : node->children) {
            collectIndices(child.second);
        }
    };

    collectIndices(current);
    return results;
}

void Trie::clear() {
    clearRecursive(root);
    root = new TrieNode();
}


void Trie::clearRecursive(TrieNode* node) {
    if (!node) return;

    for (auto& child : node->children) {
        clearRecursive(child.second);
    }

    delete node;
}

