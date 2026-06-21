#ifndef INDEXBST_H
#define INDEXBST_H

#include <string>
#include <vector>

class BSTNode {
public:
    std::string key;
    size_t offset;
    BSTNode* left;
    BSTNode* right;

    BSTNode(const std::string& k, size_t off);
};

class IndexBST {
private:
    BSTNode* root;

    BSTNode* insertNode(BSTNode* node, const std::string& key, size_t offset, bool& success);
    long long searchNode(BSTNode* node, const std::string& key);
    BSTNode* removeNode(BSTNode* node, const std::string& key);
    BSTNode* findMin(BSTNode* node);
    void destroyTree(BSTNode* node);

public:
    IndexBST();
    ~IndexBST();

    bool insert(const std::string& key, size_t offset);
    long long search(const std::string& key);
    void remove(const std::string& key);
};

#endif