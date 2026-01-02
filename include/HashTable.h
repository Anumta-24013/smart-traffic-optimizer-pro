/**
 * Smart Traffic Route Optimizer
 * Hash Table Data Structure Implementation
 * 
 * A hash table for O(1) average case junction lookup by ID
 * Uses chaining for collision resolution
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <stdexcept>

template <typename K, typename V>
class HashTable {
private:
    struct HashNode {
        K key;
        V value;
        HashNode(const K& k, const V& v) : key(k), value(v) {}
    };

    std::vector<std::list<HashNode>> buckets;
    size_t numElements;
    size_t numBuckets;
    float maxLoadFactor;

    // Hash function for strings
    size_t hashString(const std::string& key) const {
        size_t hash = 5381;
        for (char c : key) {
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
        return hash;
    }

    // Hash function for integers
    size_t hashInt(int key) const {
        return static_cast<size_t>(key);
    }

    // Generic hash function
    size_t getHash(const K& key) const {
        if constexpr (std::is_same_v<K, std::string>) {
            return hashString(key) % numBuckets;
        } else if constexpr (std::is_integral_v<K>) {
            return hashInt(static_cast<int>(key)) % numBuckets;
        } else {
            // Use std::hash as fallback
            return std::hash<K>{}(key) % numBuckets;
        }
    }

    // Rehash when load factor exceeds threshold
    void rehash() {
        size_t newSize = numBuckets * 2;
        std::vector<std::list<HashNode>> newBuckets(newSize);
        
        for (auto& bucket : buckets) {
            for (auto& node : bucket) {
                size_t newIndex;
                if constexpr (std::is_same_v<K, std::string>) {
                    newIndex = hashString(node.key) % newSize;
                } else if constexpr (std::is_integral_v<K>) {
                    newIndex = hashInt(static_cast<int>(node.key)) % newSize;
                } else {
                    newIndex = std::hash<K>{}(node.key) % newSize;
                }
                newBuckets[newIndex].push_back(node);
            }
        }
        
        buckets = std::move(newBuckets);
        numBuckets = newSize;
    }

public:
    HashTable(size_t initialSize = 16, float loadFactor = 0.75f) 
        : numBuckets(initialSize), numElements(0), maxLoadFactor(loadFactor) {
        buckets.resize(numBuckets);
    }

    // Add these methods to HashTable class (public section):

    struct HashTableMetrics {
        size_t elementCount;
        size_t bucketCount;
        float loadFactor;
        size_t longestChain;
        double avgChainLength;
        size_t collisions;
        size_t rehashes;
        size_t memoryUsageBytes;
        size_t searchOps;
    };

    HashTableMetrics getMetrics() const {
        HashTableMetrics m;
        m.elementCount = numElements;
        m.bucketCount = numBuckets;
        m.loadFactor = static_cast<float>(numElements) / numBuckets;
        
        // Calculate chain statistics
        m.longestChain = 0;
        size_t totalChainLength = 0;
        m.collisions = 0;
        
        for (const auto& bucket : buckets) {
            size_t chainLen = bucket.size();
            if (chainLen > 1) {
                m.collisions += (chainLen - 1);
            }
            if (chainLen > m.longestChain) {
                m.longestChain = chainLen;
            }
            totalChainLength += chainLen;
        }
        
        m.avgChainLength = numElements > 0 ? 
                        static_cast<double>(totalChainLength) / numBuckets : 0.0;
        
        m.rehashes = 0; // Track this if you add rehash counter
        m.memoryUsageBytes = numBuckets * sizeof(std::list<HashNode>) + 
                            numElements * (sizeof(K) + sizeof(V) + 32);
        m.searchOps = 0;
        
        return m;
    }

    float getLoadFactor() const {
        return static_cast<float>(numElements) / numBuckets;
    }

    // Insert or update a key-value pair - O(1) average
    void insert(const K& key, const V& value) {
        // Check load factor and rehash if needed
        if (static_cast<float>(numElements + 1) / numBuckets > maxLoadFactor) {
            rehash();
        }

        size_t index = getHash(key);
        
        // Check if key already exists
        for (auto& node : buckets[index]) {
            if (node.key == key) {
                node.value = value; // Update existing
                return;
            }
        }
        
        // Insert new
        buckets[index].push_back(HashNode(key, value));
        numElements++;
    }

    // Search for a key - O(1) average
    bool search(const K& key, V* result = nullptr) const {
        size_t index = getHash(key);
        
        for (const auto& node : buckets[index]) {
            if (node.key == key) {
                if (result) *result = node.value;
                return true;
            }
        }
        return false;
    }

    // Get value by key (throws if not found)
    V& get(const K& key) {
        size_t index = getHash(key);
        
        for (auto& node : buckets[index]) {
            if (node.key == key) {
                return node.value;
            }
        }
        throw std::runtime_error("Key not found in hash table");
    }

    // Get value by key (const version)
    const V& get(const K& key) const {
        size_t index = getHash(key);
        
        for (const auto& node : buckets[index]) {
            if (node.key == key) {
                return node.value;
            }
        }
        throw std::runtime_error("Key not found in hash table");
    }

    // Operator[] for convenient access
    V& operator[](const K& key) {
        size_t index = getHash(key);
        
        for (auto& node : buckets[index]) {
            if (node.key == key) {
                return node.value;
            }
        }
        
        // Insert default value if not found
        if (static_cast<float>(numElements + 1) / numBuckets > maxLoadFactor) {
            rehash();
            index = getHash(key);
        }
        buckets[index].push_back(HashNode(key, V()));
        numElements++;
        return buckets[index].back().value;
    }

    // Remove a key - O(1) average
    bool remove(const K& key) {
        size_t index = getHash(key);
        
        for (auto it = buckets[index].begin(); it != buckets[index].end(); ++it) {
            if (it->key == key) {
                buckets[index].erase(it);
                numElements--;
                return true;
            }
        }
        return false;
    }

    // Check if key exists
    bool contains(const K& key) const {
        return search(key);
    }

    // Get number of elements
    size_t size() const { return numElements; }

    // Check if empty
    bool isEmpty() const { return numElements == 0; }

    // Clear all elements
    void clear() {
        for (auto& bucket : buckets) {
            bucket.clear();
        }
        numElements = 0;
    }

    // Get all keys
    std::vector<K> keys() const {
        std::vector<K> result;
        result.reserve(numElements);
        for (const auto& bucket : buckets) {
            for (const auto& node : bucket) {
                result.push_back(node.key);
            }
        }
        return result;
    }

    // Get all values
    std::vector<V> values() const {
        std::vector<V> result;
        result.reserve(numElements);
        for (const auto& bucket : buckets) {
            for (const auto& node : bucket) {
                result.push_back(node.value);
            }
        }
        return result;
    }

    // Get all key-value pairs
    std::vector<std::pair<K, V>> getAll() const {
        std::vector<std::pair<K, V>> result;
        result.reserve(numElements);
        for (const auto& bucket : buckets) {
            for (const auto& node : bucket) {
                result.push_back({node.key, node.value});
            }
        }
        return result;
    }

    // Iterate over all elements
    void forEach(std::function<void(const K&, V&)> callback) {
        for (auto& bucket : buckets) {
            for (auto& node : bucket) {
                callback(node.key, node.value);
            }
        }
    }

    // Get statistics
    void printStats() const {
        std::cout << "Hash Table Statistics:\n";
        std::cout << "  Elements: " << numElements << "\n";
        std::cout << "  Buckets: " << numBuckets << "\n";
        std::cout << "  Load Factor: " << static_cast<float>(numElements) / numBuckets << "\n";
        
        size_t maxChain = 0;
        size_t emptyBuckets = 0;
        for (const auto& bucket : buckets) {
            if (bucket.empty()) emptyBuckets++;
            if (bucket.size() > maxChain) maxChain = bucket.size();
        }
        std::cout << "  Empty Buckets: " << emptyBuckets << "\n";
        std::cout << "  Max Chain Length: " << maxChain << "\n";
    }
};

#endif // HASHTABLE_H
