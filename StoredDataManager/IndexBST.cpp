#include "IndexBST.h"

BSTNode::BSTNode(const std::string& k, size_t off) {
    key = k;
    offset = off;
    left = nullptr;
    right = nullptr;
}

IndexBST::IndexBST() {
    root = nullptr;
}

IndexBST::~IndexBST() {
    destroyTree(root);
}

void IndexBST::destroyTree(BSTNode* node) {
    if (node != nullptr) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

bool IndexBST::insert(const std::string& key, size_t offset) {
    bool success = false;
    root = insertNode(root, key, offset, success);
    return success;
}

BSTNode* IndexBST::insertNode(BSTNode* node, const std::string& key, size_t offset, bool& success) {
    if (node == nullptr) {
        success = true;
        return new BSTNode(key, offset);
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key, offset, success);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, offset, success);
    } else {
        success = false;
    }

    return node;
}

long long IndexBST::search(const std::string& key) {
    return searchNode(root, key);
}

long long IndexBST::searchNode(BSTNode* node, const std::string& key) {
    if (node == nullptr) {
        return -1;
    }

    if (key == node->key) {
        return node->offset;
    } else if (key < node->key) {
        return searchNode(node->left, key);
    } else {
        return searchNode(node->right, key);
    }
}

void IndexBST::remove(const std::string& key) {
    root = removeNode(root, key);
}

BSTNode* IndexBST::findMin(BSTNode* node) {
    while (node && node->left != nullptr) {
        node = node->left;
    }
    return node;
}

BSTNode* IndexBST::removeNode(BSTNode* node, const std::string& key) {
    if (node == nullptr) {
        return node;
    }

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        if (node->left == nullptr && node->right == nullptr) {
            delete node;
            return nullptr;
        } else if (node->left == nullptr) {
            BSTNode* temp = node->right;
            delete node;
            return temp;
        } else if (node->right == nullptr) {
            BSTNode* temp = node->left;
            delete node;
            return temp;
        }

        BSTNode* temp = findMin(node->right);
        node->key = temp->key;
        node->offset = temp->offset;
        node->right = removeNode(node->right, temp->key);
    }

    return node;
}