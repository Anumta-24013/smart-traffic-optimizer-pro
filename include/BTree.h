/**
 * Smart Traffic Route Optimizer
 * B-Tree Data Structure Implementation
 * 
 * A self-balancing tree data structure for efficient junction indexing
 * Time Complexity: O(log n) for search, insert, delete
 */

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <vector>
#include <string>
#include <functional>

template <typename K, typename V>
class BTreeNode {
public:
    std::vector<K> keys;
    std::vector<V> values;
    std::vector<BTreeNode<K, V>*> children;
    bool isLeaf;
    int minDegree; // Minimum degree (t)

    BTreeNode(int t, bool leaf) : minDegree(t), isLeaf(leaf) {}

    ~BTreeNode() {
        for (auto child : children) {
            delete child;
        }
    }

    // Find index of first key >= k
    int findKey(const K& k) const {
        int idx = 0;
        while (idx < keys.size() && keys[idx] < k) {
            ++idx;
        }
        return idx;
    }

    // Insert a new key in non-full node
    void insertNonFull(const K& k, const V& v);

    // Split child y of this node
    void splitChild(int i, BTreeNode<K, V>* y);

    // Traverse all nodes in subtree
    void traverse(std::function<void(const K&, const V&)> callback) const;

    // Search key in subtree
    BTreeNode<K, V>* search(const K& k, V* result) const;

    // Remove key from subtree
    void remove(const K& k);

    // Helper functions for remove
    void removeFromLeaf(int idx);
    void removeFromNonLeaf(int idx);
    K getPredecessor(int idx);
    K getSuccessor(int idx);
    void fill(int idx);
    void borrowFromPrev(int idx);
    void borrowFromNext(int idx);
    void merge(int idx);
};

template <typename K, typename V>
class BTree {
private:
    BTreeNode<K, V>* root;
    int minDegree; // Minimum degree

public:
    BTree(int degree = 3) : root(nullptr), minDegree(degree) {}

    ~BTree() {
        delete root;
    }

    // Traverse the tree
    void traverse(std::function<void(const K&, const V&)> callback) const {
        if (root != nullptr) {
            root->traverse(callback);
        }
    }

    // Search a key in the tree
    bool search(const K& k, V* result = nullptr) const {
        if (root == nullptr) return false;
        return root->search(k, result) != nullptr;
    }

    // Insert a new key-value pair
    void insert(const K& k, const V& v);

    // Remove a key
    void remove(const K& k);

    // Check if tree is empty
    bool isEmpty() const { return root == nullptr; }

    // Get all key-value pairs as vector
    std::vector<std::pair<K, V>> getAll() const {
        std::vector<std::pair<K, V>> result;
        traverse([&result](const K& key, const V& value) {
            result.push_back({key, value});
        });
        return result;
    }

    // Get count of elements
    size_t size() const {
        size_t count = 0;
        traverse([&count](const K&, const V&) { ++count; });
        return count;
    }

    // Add these methods to BTree class (public section):

    // ============ METRICS ============
    struct BTreeMetrics {
        int height;
        int nodeCount;
        int elementCount;
        double avgKeysPerNode;
        size_t memoryBytes;
        int searchOps;
    };

    BTreeMetrics getMetrics() const {
        BTreeMetrics m;
        m.elementCount = size();
        m.height = getHeight();
        m.nodeCount = countNodes();
        m.avgKeysPerNode = m.nodeCount > 0 ? 
                        static_cast<double>(m.elementCount) / m.nodeCount : 0;
        m.memoryBytes = m.elementCount * (sizeof(K) + sizeof(V) + 64); // Approx
        m.searchOps = 0;
        return m;
    }

    int getHeight() const {
        return root ? getHeightHelper(root) : 0;
    }

    int countNodes() const {
        return root ? countNodesHelper(root) : 0;
    }

    // ============ RANGE QUERY ============
    std::vector<std::pair<K, V>> rangeQuery(const K& minKey, const K& maxKey) const {
        std::vector<std::pair<K, V>> results;
        if (!root) return results;
        
        rangeQueryHelper(root, minKey, maxKey, results);
        return results;
    }

    // ============ PREFIX SEARCH ============
    std::vector<std::pair<K, V>> prefixSearch(const K& prefix) const {
        std::vector<std::pair<K, V>> results;
        if (!root) return results;
        
        // For string keys, find all keys starting with prefix
        traverse([&](const K& key, const V& value) {
            if constexpr (std::is_same_v<K, std::string>) {
                if (key.find(prefix) == 0) {  // Starts with prefix
                    results.push_back({key, value});
                }
            }
        });
        
        return results;
    }

    private:
    // Helper for getHeight
    int getHeightHelper(BTreeNode<K, V>* node) const {
        if (!node || node->isLeaf) return 1;
        
        int maxChildHeight = 0;
        for (auto child : node->children) {
            maxChildHeight = std::max(maxChildHeight, getHeightHelper(child));
        }
        return 1 + maxChildHeight;
    }

    // Helper for countNodes
    int countNodesHelper(BTreeNode<K, V>* node) const {
        if (!node) return 0;
        
        int count = 1;
        for (auto child : node->children) {
            count += countNodesHelper(child);
        }
        return count;
    }

    // Helper for range query
    void rangeQueryHelper(BTreeNode<K, V>* node, const K& minKey, const K& maxKey,
                        std::vector<std::pair<K, V>>& results) const {
        if (!node) return;
        
        size_t i = 0;
        while (i < node->keys.size() && node->keys[i] < minKey) {
            i++;
        }
        
        while (i < node->keys.size() && node->keys[i] <= maxKey) {
            if (!node->isLeaf && i < node->children.size()) {
                rangeQueryHelper(node->children[i], minKey, maxKey, results);
            }
            
            if (node->keys[i] >= minKey && node->keys[i] <= maxKey) {
                results.push_back({node->keys[i], node->values[i]});
            }
            i++;
        }
        
        if (!node->isLeaf && i < node->children.size()) {
            rangeQueryHelper(node->children[i], minKey, maxKey, results);
        }
    }
};

// ======================== Implementation ========================

template <typename K, typename V>
void BTreeNode<K, V>::traverse(std::function<void(const K&, const V&)> callback) const {
    int i;
    for (i = 0; i < keys.size(); i++) {
        if (!isLeaf) {
            children[i]->traverse(callback);
        }
        callback(keys[i], values[i]);
    }
    if (!isLeaf) {
        children[i]->traverse(callback);
    }
}

template <typename K, typename V>
BTreeNode<K, V>* BTreeNode<K, V>::search(const K& k, V* result) const {
    int i = findKey(k);
    
    if (i < static_cast<int>(keys.size()) && keys[i] == k) {
        if (result) *result = values[i];
        return const_cast<BTreeNode<K, V>*>(this);
    }
    
    if (isLeaf) return nullptr;
    
    return children[i]->search(k, result);
}

template <typename K, typename V>
void BTree<K, V>::insert(const K& k, const V& v) {
    if (root == nullptr) {
        root = new BTreeNode<K, V>(minDegree, true);
        root->keys.push_back(k);
        root->values.push_back(v);
    } else {
        // Check if key already exists - update value
        V existing;
        if (search(k, &existing)) {
            // Update existing key
            std::function<void(BTreeNode<K, V>*)> updateNode = [&](BTreeNode<K, V>* node) {
                int i = node->findKey(k);
                if (i < node->keys.size() && node->keys[i] == k) {
                    node->values[i] = v;
                    return;
                }
                if (!node->isLeaf) {
                    updateNode(node->children[i]);
                }
            };
            updateNode(root);
            return;
        }

        // If root is full, split it
        if (root->keys.size() == 2 * minDegree - 1) {
            BTreeNode<K, V>* newRoot = new BTreeNode<K, V>(minDegree, false);
            newRoot->children.push_back(root);
            newRoot->splitChild(0, root);
            
            // Decide which child gets new key
            int i = 0;
            if (newRoot->keys[0] < k) i++;
            newRoot->children[i]->insertNonFull(k, v);
            
            root = newRoot;
        } else {
            root->insertNonFull(k, v);
        }
    }
}

template <typename K, typename V>
void BTreeNode<K, V>::insertNonFull(const K& k, const V& v) {
    int i = keys.size() - 1;
    
    if (isLeaf) {
        // Find location and insert
        keys.push_back(K());
        values.push_back(V());
        while (i >= 0 && keys[i] > k) {
            keys[i + 1] = keys[i];
            values[i + 1] = values[i];
            i--;
        }
        keys[i + 1] = k;
        values[i + 1] = v;
    } else {
        // Find child to insert into
        while (i >= 0 && keys[i] > k) i--;
        i++;
        
        // Split child if full
        if (children[i]->keys.size() == 2 * minDegree - 1) {
            splitChild(i, children[i]);
            if (keys[i] < k) i++;
        }
        children[i]->insertNonFull(k, v);
    }
}

template <typename K, typename V>
void BTreeNode<K, V>::splitChild(int i, BTreeNode<K, V>* y) {
    BTreeNode<K, V>* z = new BTreeNode<K, V>(y->minDegree, y->isLeaf);
    
    // Copy last (t-1) keys and values to z
    for (int j = 0; j < minDegree - 1; j++) {
        z->keys.push_back(y->keys[j + minDegree]);
        z->values.push_back(y->values[j + minDegree]);
    }
    
    // Copy last t children to z if not leaf
    if (!y->isLeaf) {
        for (int j = 0; j < minDegree; j++) {
            z->children.push_back(y->children[j + minDegree]);
        }
    }
    
    // Insert new child into this node
    children.insert(children.begin() + i + 1, z);
    
    // Move middle key up to this node
    keys.insert(keys.begin() + i, y->keys[minDegree - 1]);
    values.insert(values.begin() + i, y->values[minDegree - 1]);
    
    // Resize y
    y->keys.resize(minDegree - 1);
    y->values.resize(minDegree - 1);
    if (!y->isLeaf) {
        y->children.resize(minDegree);
    }
}

template <typename K, typename V>
void BTree<K, V>::remove(const K& k) {
    if (root == nullptr) return;
    
    root->remove(k);
    
    // If root has no keys, make first child the new root
    if (root->keys.empty()) {
        BTreeNode<K, V>* tmp = root;
        if (root->isLeaf) {
            root = nullptr;
        } else {
            root = root->children[0];
            tmp->children.clear(); // Prevent deletion of new root
        }
        delete tmp;
    }
}

template <typename K, typename V>
void BTreeNode<K, V>::remove(const K& k) {
    int idx = findKey(k);
    
    // Key is in this node
    if (idx < keys.size() && keys[idx] == k) {
        if (isLeaf) {
            removeFromLeaf(idx);
        } else {
            removeFromNonLeaf(idx);
        }
    } else {
        if (isLeaf) {
            return; // Key not found
        }
        
        bool isLastChild = (idx == keys.size());
        
        if (children[idx]->keys.size() < minDegree) {
            fill(idx);
        }
        
        if (isLastChild && idx > keys.size()) {
            children[idx - 1]->remove(k);
        } else {
            children[idx]->remove(k);
        }
    }
}

template <typename K, typename V>
void BTreeNode<K, V>::removeFromLeaf(int idx) {
    keys.erase(keys.begin() + idx);
    values.erase(values.begin() + idx);
}

template <typename K, typename V>
void BTreeNode<K, V>::removeFromNonLeaf(int idx) {
    K k = keys[idx];
    
    if (children[idx]->keys.size() >= minDegree) {
        // Get predecessor
        BTreeNode<K, V>* cur = children[idx];
        while (!cur->isLeaf) {
            cur = cur->children[cur->keys.size()];
        }
        keys[idx] = cur->keys[cur->keys.size() - 1];
        values[idx] = cur->values[cur->values.size() - 1];
        children[idx]->remove(keys[idx]);
    } else if (children[idx + 1]->keys.size() >= minDegree) {
        // Get successor
        BTreeNode<K, V>* cur = children[idx + 1];
        while (!cur->isLeaf) {
            cur = cur->children[0];
        }
        keys[idx] = cur->keys[0];
        values[idx] = cur->values[0];
        children[idx + 1]->remove(keys[idx]);
    } else {
        merge(idx);
        children[idx]->remove(k);
    }
}

template <typename K, typename V>
void BTreeNode<K, V>::fill(int idx) {
    if (idx != 0 && children[idx - 1]->keys.size() >= minDegree) {
        borrowFromPrev(idx);
    } else if (idx != keys.size() && children[idx + 1]->keys.size() >= minDegree) {
        borrowFromNext(idx);
    } else {
        if (idx != keys.size()) {
            merge(idx);
        } else {
            merge(idx - 1);
        }
    }
}

template <typename K, typename V>
void BTreeNode<K, V>::borrowFromPrev(int idx) {
    BTreeNode<K, V>* child = children[idx];
    BTreeNode<K, V>* sibling = children[idx - 1];
    
    child->keys.insert(child->keys.begin(), keys[idx - 1]);
    child->values.insert(child->values.begin(), values[idx - 1]);
    
    if (!child->isLeaf) {
        child->children.insert(child->children.begin(), sibling->children.back());
        sibling->children.pop_back();
    }
    
    keys[idx - 1] = sibling->keys.back();
    values[idx - 1] = sibling->values.back();
    sibling->keys.pop_back();
    sibling->values.pop_back();
}

template <typename K, typename V>
void BTreeNode<K, V>::borrowFromNext(int idx) {
    BTreeNode<K, V>* child = children[idx];
    BTreeNode<K, V>* sibling = children[idx + 1];
    
    child->keys.push_back(keys[idx]);
    child->values.push_back(values[idx]);
    
    if (!child->isLeaf) {
        child->children.push_back(sibling->children[0]);
        sibling->children.erase(sibling->children.begin());
    }
    
    keys[idx] = sibling->keys[0];
    values[idx] = sibling->values[0];
    sibling->keys.erase(sibling->keys.begin());
    sibling->values.erase(sibling->values.begin());
}

template <typename K, typename V>
void BTreeNode<K, V>::merge(int idx) {
    BTreeNode<K, V>* child = children[idx];
    BTreeNode<K, V>* sibling = children[idx + 1];
    
    child->keys.push_back(keys[idx]);
    child->values.push_back(values[idx]);
    
    for (int i = 0; i < sibling->keys.size(); ++i) {
        child->keys.push_back(sibling->keys[i]);
        child->values.push_back(sibling->values[i]);
    }
    
    if (!child->isLeaf) {
        for (int i = 0; i <= sibling->keys.size(); ++i) {
            child->children.push_back(sibling->children[i]);
        }
    }
    
    keys.erase(keys.begin() + idx);
    values.erase(values.begin() + idx);
    children.erase(children.begin() + idx + 1);
    
    sibling->children.clear(); // Prevent double delete
    delete sibling;
}

#endif // BTREE_H
