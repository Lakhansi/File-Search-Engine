// src/Trie.cpp
#include "Trie.h"
#include <iostream>
#include <functional> // ADD THIS LINE

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

    // Traverse to the end of the prefix
    for (char c : prefix) {
        if (current->children.find(c) == current->children.end()) {
            return results; // Prefix not found, return empty
        }
        current = current->children[c];
    }

    // Collect all file indices from this node and all its descendants
    std::function<void(TrieNode*)> collectIndices = [&](TrieNode* node) {
        if (!node) return;

        // Add indices from current node
        results.insert(results.end(), node->fileIndices.begin(), node->fileIndices.end());

        // Recursively collect from all children
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
