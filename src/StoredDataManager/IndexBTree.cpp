#include "IndexBTree.h"

using namespace std;

BTreeNode::BTreeNode(bool isLeaf) {
    leaf = isLeaf;
}

IndexBTree::IndexBTree(int degree) {
    root = nullptr;
    t = degree;
}

IndexBTree::~IndexBTree() {
    destroyTree(root);
}

void IndexBTree::destroyTree(BTreeNode* node) {
    if (node != nullptr) {
        for (BTreeNode* child : node->children) {
            destroyTree(child);
        }
        delete node;
    }
}

long long IndexBTree::search(const string& key) {
    return searchNode(root, key);
}

long long IndexBTree::searchNode(BTreeNode* node, const string& key) {
    if (node == nullptr) return -1;
    int i = 0;
    while (i < node->keys.size() && key > node->keys[i]) i++;
    if (i < node->keys.size() && key == node->keys[i]) return node->offsets[i];
    if (node->leaf) return -1;
    return searchNode(node->children[i], key);
}

bool IndexBTree::insert(const string& key, size_t offset) {
    if (search(key) != -1) return false;

    if (root == nullptr) {
        root = new BTreeNode(true);
        root->keys.push_back(key);
        root->offsets.push_back(offset);
        return true;
    }

    if (root->keys.size() == 2 * t - 1) {
        BTreeNode* s = new BTreeNode(false);
        s->children.push_back(root);
        splitChild(s, 0, root);

        int i = 0;
        if (s->keys[0] < key) i++;
        insertNonFull(s->children[i], key, offset);
        root = s;
    } else {
        insertNonFull(root, key, offset);
    }
    return true;
}

void IndexBTree::insertNonFull(BTreeNode* node, const string& key, size_t offset) {
    int i = node->keys.size() - 1;

    if (node->leaf) {
        node->keys.push_back("");
        node->offsets.push_back(0);

        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            node->offsets[i + 1] = node->offsets[i];
            i--;
        }

        node->keys[i + 1] = key;
        node->offsets[i + 1] = offset;
    } else {
        while (i >= 0 && node->keys[i] > key) i--;
        i++;

        if (node->children[i]->keys.size() == 2 * t - 1) {
            splitChild(node, i, node->children[i]);
            if (node->keys[i] < key) i++;
        }
        insertNonFull(node->children[i], key, offset);
    }
}

void IndexBTree::splitChild(BTreeNode* parent, int i, BTreeNode* child) {
    BTreeNode* z = new BTreeNode(child->leaf);

    for (int j = 0; j < t - 1; j++) {
        z->keys.push_back(child->keys[j + t]);
        z->offsets.push_back(child->offsets[j + t]);
    }

    if (!child->leaf) {
        for (int j = 0; j < t; j++) {
            z->children.push_back(child->children[j + t]);
        }
    }

    child->keys.resize(t - 1);
    child->offsets.resize(t - 1);
    if (!child->leaf) child->children.resize(t);

    parent->children.insert(parent->children.begin() + i + 1, z);
    parent->keys.insert(parent->keys.begin() + i, child->keys[t - 1]);
    parent->offsets.insert(parent->offsets.begin() + i, child->offsets[t - 1]);
}

void IndexBTree::remove(const string& key) {
    if (!root) return;
    removeNode(root, key);

    if (root->keys.empty()) {
        BTreeNode* tmp = root;
        if (root->leaf) {
            root = nullptr;
        } else {
            root = root->children[0];
        }
        delete tmp;
    }
}

int IndexBTree::findKey(BTreeNode* node, const string& key) {
    int idx = 0;
    while (idx < node->keys.size() && node->keys[idx] < key) ++idx;
    return idx;
}

void IndexBTree::removeNode(BTreeNode* node, const string& key) {
    int idx = findKey(node, key);

    if (idx < node->keys.size() && node->keys[idx] == key) {
        if (node->leaf) {
            removeFromLeaf(node, idx);
        } else {
            removeFromNonLeaf(node, idx);
        }
    } else {
        if (node->leaf) return;

        bool flag = (idx == node->keys.size());

        if (node->children[idx]->keys.size() < t) {
            fill(node, idx);
        }

        if (flag && idx > node->keys.size()) {
            removeNode(node->children[idx - 1], key);
        } else {
            removeNode(node->children[idx], key);
        }
    }
}

void IndexBTree::removeFromLeaf(BTreeNode* node, int idx) {
    node->keys.erase(node->keys.begin() + idx);
    node->offsets.erase(node->offsets.begin() + idx);
}

void IndexBTree::removeFromNonLeaf(BTreeNode* node, int idx) {
    string k = node->keys[idx];

    if (node->children[idx]->keys.size() >= t) {
        string predKey;
        size_t predOff;
        getPredecessor(node, idx, predKey, predOff);
        node->keys[idx] = predKey;
        node->offsets[idx] = predOff;
        removeNode(node->children[idx], predKey);
    }
    else if (node->children[idx + 1]->keys.size() >= t) {
        string succKey;
        size_t succOff;
        getSuccessor(node, idx, succKey, succOff);
        node->keys[idx] = succKey;
        node->offsets[idx] = succOff;
        removeNode(node->children[idx + 1], succKey);
    }
    else {
        merge(node, idx);
        removeNode(node->children[idx], k);
    }
}

void IndexBTree::getPredecessor(BTreeNode* node, int idx, string& key, size_t& offset) {
    BTreeNode* cur = node->children[idx];
    while (!cur->leaf) {
        cur = cur->children.back();
    }
    key = cur->keys.back();
    offset = cur->offsets.back();
}

void IndexBTree::getSuccessor(BTreeNode* node, int idx, string& key, size_t& offset) {
    BTreeNode* cur = node->children[idx + 1];
    while (!cur->leaf) {
        cur = cur->children.front();
    }
    key = cur->keys.front();
    offset = cur->offsets.front();
}

void IndexBTree::fill(BTreeNode* node, int idx) {
    if (idx != 0 && node->children[idx - 1]->keys.size() >= t) {
        borrowFromPrev(node, idx);
    } 
    else if (idx != node->keys.size() && node->children[idx + 1]->keys.size() >= t) {
        borrowFromNext(node, idx);
    } 
    else {
        if (idx != node->keys.size()) {
            merge(node, idx);
        } else {
            merge(node, idx - 1);
        }
    }
}

void IndexBTree::borrowFromPrev(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];

    child->keys.insert(child->keys.begin(), node->keys[idx - 1]);
    child->offsets.insert(child->offsets.begin(), node->offsets[idx - 1]);
    
    if (!child->leaf) {
        child->children.insert(child->children.begin(), sibling->children.back());
    }

    node->keys[idx - 1] = sibling->keys.back();
    node->offsets[idx - 1] = sibling->offsets.back();

    sibling->keys.pop_back();
    sibling->offsets.pop_back();
    
    if (!sibling->leaf) {
        sibling->children.pop_back();
    }
}

void IndexBTree::borrowFromNext(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    child->keys.push_back(node->keys[idx]);
    child->offsets.push_back(node->offsets[idx]);
    
    if (!child->leaf) {
        child->children.push_back(sibling->children.front());
    }

    node->keys[idx] = sibling->keys.front();
    node->offsets[idx] = sibling->offsets.front();

    sibling->keys.erase(sibling->keys.begin());
    sibling->offsets.erase(sibling->offsets.begin());
    
    if (!sibling->leaf) {
        sibling->children.erase(sibling->children.begin());
    }
}

void IndexBTree::merge(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    child->keys.push_back(node->keys[idx]);
    child->offsets.push_back(node->offsets[idx]);

    for (int i = 0; i < sibling->keys.size(); ++i) {
        child->keys.push_back(sibling->keys[i]);
        child->offsets.push_back(sibling->offsets[i]);
    }
    
    if (!child->leaf) {
        for (int i = 0; i < sibling->children.size(); ++i) {
            child->children.push_back(sibling->children[i]);
        }
    }

    node->keys.erase(node->keys.begin() + idx);
    node->offsets.erase(node->offsets.begin() + idx);
    node->children.erase(node->children.begin() + idx + 1);

    delete sibling;
}