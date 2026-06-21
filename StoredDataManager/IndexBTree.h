#ifndef INDEXBTREE_H
#define INDEXBTREE_H

#include <string>
#include <vector>

class BTreeNode {
public:
    bool leaf;
    std::vector<std::string> keys;
    std::vector<size_t> offsets;
    std::vector<BTreeNode*> children;

    BTreeNode(bool isLeaf);
};

class IndexBTree {
private:
    BTreeNode* root;
    int t;

    void splitChild(BTreeNode* parent, int i, BTreeNode* child);
    void insertNonFull(BTreeNode* node, const std::string& key, size_t offset);
    long long searchNode(BTreeNode* node, const std::string& key);
    void removeNode(BTreeNode* node, const std::string& key);
    
    int findKey(BTreeNode* node, const std::string& key);
    void removeFromLeaf(BTreeNode* node, int idx);
    void removeFromNonLeaf(BTreeNode* node, int idx);
    void getPredecessor(BTreeNode* node, int idx, std::string& key, size_t& offset);
    void getSuccessor(BTreeNode* node, int idx, std::string& key, size_t& offset);
    void fill(BTreeNode* node, int idx);
    void borrowFromPrev(BTreeNode* node, int idx);
    void borrowFromNext(BTreeNode* node, int idx);
    void merge(BTreeNode* node, int idx);

public:
    IndexBTree(int degree = 3);
    ~IndexBTree();
    void destroyTree(BTreeNode* node);

    bool insert(const std::string& key, size_t offset);
    long long search(const std::string& key);
    void remove(const std::string& key);
};

#endif