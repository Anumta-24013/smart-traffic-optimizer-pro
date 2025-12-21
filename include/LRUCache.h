/**
 * Smart Traffic Route Optimizer
 * LRU Cache Implementation
 * 
 * Least Recently Used cache for route calculations
 * Time Complexity: O(1) for get and put operations
 */

#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <unordered_map>
#include <list>
#include <stdexcept>
#include <functional>

template <typename K, typename V>
class LRUCache {
private:
    struct CacheNode {
        K key;
        V value;
        CacheNode(const K& k, const V& v) : key(k), value(v) {}
    };

    size_t capacity;
    size_t hits;
    size_t misses;
    
    // Doubly linked list for LRU ordering (front = most recent)
    std::list<CacheNode> cacheList;
    
    // HashMap for O(1) access to list nodes
    std::unordered_map<K, typename std::list<CacheNode>::iterator> cacheMap;

public:
    explicit LRUCache(size_t cap = 100) : capacity(cap), hits(0), misses(0) {}

    // Get value from cache - O(1)
    bool get(const K& key, V* result = nullptr) {
        auto it = cacheMap.find(key);
        if (it == cacheMap.end()) {
            misses++;
            return false;
        }

        hits++;
        // Move to front (most recently used)
        cacheList.splice(cacheList.begin(), cacheList, it->second);
        
        if (result) {
            *result = it->second->value;
        }
        return true;
    }

    // Get value or throw - O(1)
    V& getValue(const K& key) {
        auto it = cacheMap.find(key);
        if (it == cacheMap.end()) {
            misses++;
            throw std::runtime_error("Key not found in cache");
        }

        hits++;
        cacheList.splice(cacheList.begin(), cacheList, it->second);
        return it->second->value;
    }

    // Put value in cache - O(1)
    void put(const K& key, const V& value) {
        auto it = cacheMap.find(key);
        
        if (it != cacheMap.end()) {
            // Update existing and move to front
            it->second->value = value;
            cacheList.splice(cacheList.begin(), cacheList, it->second);
            return;
        }

        // Remove LRU if at capacity
        if (cacheList.size() >= capacity) {
            auto& lru = cacheList.back();
            cacheMap.erase(lru.key);
            cacheList.pop_back();
        }

        // Insert new at front
        cacheList.push_front(CacheNode(key, value));
        cacheMap[key] = cacheList.begin();
    }

    // Check if key exists - O(1)
    bool contains(const K& key) const {
        return cacheMap.find(key) != cacheMap.end();
    }

    // Remove key from cache - O(1)
    bool remove(const K& key) {
        auto it = cacheMap.find(key);
        if (it == cacheMap.end()) {
            return false;
        }

        cacheList.erase(it->second);
        cacheMap.erase(it);
        return true;
    }

    // Clear all entries
    void clear() {
        cacheList.clear();
        cacheMap.clear();
        hits = 0;
        misses = 0;
    }

    // Get current size
    size_t size() const { return cacheList.size(); }

    // Check if empty
    bool isEmpty() const { return cacheList.empty(); }

    // Check if full
    bool isFull() const { return cacheList.size() >= capacity; }

    // Get capacity
    size_t getCapacity() const { return capacity; }

    // Set new capacity
    void setCapacity(size_t newCapacity) {
        capacity = newCapacity;
        while (cacheList.size() > capacity) {
            auto& lru = cacheList.back();
            cacheMap.erase(lru.key);
            cacheList.pop_back();
        }
    }

    // Get cache statistics
    size_t getHits() const { return hits; }
    size_t getMisses() const { return misses; }
    
    double getHitRate() const {
        size_t total = hits + misses;
        return total > 0 ? static_cast<double>(hits) / total * 100.0 : 0.0;
    }

    // Reset statistics
    void resetStats() {
        hits = 0;
        misses = 0;
    }

    // Get all keys (in LRU order, most recent first)
    std::vector<K> keys() const {
        std::vector<K> result;
        result.reserve(cacheList.size());
        for (const auto& node : cacheList) {
            result.push_back(node.key);
        }
        return result;
    }

    // Get all entries (in LRU order)
    std::vector<std::pair<K, V>> getAll() const {
        std::vector<std::pair<K, V>> result;
        result.reserve(cacheList.size());
        for (const auto& node : cacheList) {
            result.push_back({node.key, node.value});
        }
        return result;
    }

    // Get or compute value - O(1) if cached
    V getOrCompute(const K& key, std::function<V()> computeFunc) {
        V result;
        if (get(key, &result)) {
            return result;
        }
        result = computeFunc();
        put(key, result);
        return result;
    }

    // Print cache statistics
    void printStats() const {
        std::cout << "LRU Cache Statistics:\n";
        std::cout << "  Size: " << size() << "/" << capacity << "\n";
        std::cout << "  Hits: " << hits << "\n";
        std::cout << "  Misses: " << misses << "\n";
        std::cout << "  Hit Rate: " << getHitRate() << "%\n";
    }
};

#endif // LRUCACHE_H
