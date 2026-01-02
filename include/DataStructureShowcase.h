/**
 * Data Structure Showcase Features
 * Demonstrates B-Tree and Hash Table POWER with 6 key features
 */

#ifndef DS_SHOWCASE_H
#define DS_SHOWCASE_H

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <fstream>
#include <sstream>
#include <cmath>
#include <set>
#include <atomic>
#include "BTree.h"
#include "HashTable.h"
#include "Models.h"

// ============ FEATURE 1: SPATIAL INDEX ============

class SpatialIndex {
private:
    BTree<double, std::vector<int>> latIndex;   // Latitude -> Junction IDs
    BTree<double, std::vector<int>> lngIndex;   // Longitude -> Junction IDs
    HashTable<int, Junction> idCache;            // Fast ID lookup
    
public:
    SpatialIndex() : latIndex(5), lngIndex(5), idCache(1024) {}
    
    void addJunction(const Junction& j) {
        // Add to spatial indices
        std::vector<int> latList;
        latIndex.search(j.latitude, &latList);
        latList.push_back(j.id);
        latIndex.insert(j.latitude, latList);
        
        std::vector<int> lngList;
        lngIndex.search(j.longitude, &lngList);
        lngList.push_back(j.id);
        lngIndex.insert(j.longitude, lngList);
        
        // Cache full junction
        idCache.insert(j.id, j);
    }
    
    // Find all junctions within radius (km)
    std::vector<Junction> findInRadius(double centerLat, double centerLng, double radiusKm) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Calculate lat/lng bounds (approximate)
        double latDelta = radiusKm / 111.0; // 1 degree lat ≈ 111 km
        double lngDelta = radiusKm / (111.0 * cos(centerLat * M_PI / 180.0));
        
        // Range query on B-Tree (O(log n + m))
        auto latCandidates = latIndex.rangeQuery(
            centerLat - latDelta, 
            centerLat + latDelta
        );
        
        std::vector<Junction> results;
        std::set<int> seen;
        
        for (const auto& [lat, ids] : latCandidates) {
            for (int id : ids) {
                if (seen.find(id) != seen.end()) continue;
                seen.insert(id);
                
                Junction j;
                if (idCache.search(id, &j)) {
                    double dist = calculateDistance(
                        centerLat, centerLng, 
                        j.latitude, j.longitude
                    );
                    if (dist <= radiusKm) {
                        results.push_back(j);
                    }
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "🎯 SPATIAL QUERY: Found " << results.size() 
                  << " junctions within " << radiusKm << "km in " 
                  << duration.count() << "ms\n";
        
        return results;
    }
    
    double calculateDistance(double lat1, double lng1, double lat2, double lng2) {
        const double R = 6371.0; // Earth radius in km
        double dLat = (lat2 - lat1) * M_PI / 180.0;
        double dLng = (lng2 - lng1) * M_PI / 180.0;
        
        double a = sin(dLat/2) * sin(dLat/2) +
                   cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
                   sin(dLng/2) * sin(dLng/2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        
        return R * c;
    }
};

// ============ FEATURE 2: PERFORMANCE MONITOR ============

class PerformanceMonitor {
private:
    struct SearchRecord {
        std::string type;
        double timeMs;
        std::chrono::system_clock::time_point timestamp;
    };
    
    std::vector<SearchRecord> history;
    std::map<std::string, std::vector<double>> timingsByType;
    
public:
    void recordSearch(const std::string& type, double timeMs) {
        SearchRecord record;
        record.type = type;
        record.timeMs = timeMs;
        record.timestamp = std::chrono::system_clock::now();
        
        history.push_back(record);
        timingsByType[type].push_back(timeMs);
        
        // Keep last 1000 records
        if (history.size() > 1000) {
            history.erase(history.begin());
        }
    }
    
    void showStats() {
        std::cout << "\n📊 PERFORMANCE STATISTICS\n";
        std::cout << "=================================\n";
        std::cout << "Total Searches: " << history.size() << "\n\n";
        
        for (const auto& [type, timings] : timingsByType) {
            if (timings.empty()) continue;
            
            double sum = 0;
            double minTime = timings[0];
            double maxTime = timings[0];
            
            for (double t : timings) {
                sum += t;
                if (t < minTime) minTime = t;
                if (t > maxTime) maxTime = t;
            }
            
            double avg = sum / timings.size();
            
            std::cout << type << " Searches:\n";
            std::cout << "  Count: " << timings.size() << "\n";
            std::cout << "  Avg: " << avg << " ms\n";
            std::cout << "  Min: " << minTime << " ms\n";
            std::cout << "  Max: " << maxTime << " ms\n\n";
        }
    }
    
    void reset() {
        history.clear();
        timingsByType.clear();
    }
};

// ============ FEATURE 3: AUTOCOMPLETE ENGINE ============

class AutocompleteEngine {
private:
    BTree<std::string, Junction> junctionTree;
    
public:
    AutocompleteEngine() : junctionTree(5) {}
    
    void addJunction(const Junction& j) {
        std::string lowerName = toLowerCase(j.name);
        junctionTree.insert(lowerName, j);
    }
    
    std::vector<Junction> search(const std::string& prefix, int maxResults = 10) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::string lowerPrefix = toLowerCase(prefix);
        auto results = junctionTree.prefixSearch(lowerPrefix);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "⚡ AUTOCOMPLETE: Found " << results.size() 
                  << " matches for \"" << prefix << "\" in " 
                  << duration.count() / 1000.0 << " ms\n";
        
        std::vector<Junction> junctions;
        for (size_t i = 0; i < results.size() && i < maxResults; i++) {
            junctions.push_back(results[i].second);
        }
        
        return junctions;
    }
    
private:
    std::string toLowerCase(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
};

// ============ FEATURE 4: STRESS TESTER ============

class StressTester {
private:
    std::mt19937 rng;
    
public:
    StressTester() : rng(std::random_device{}()) {}
    
    // Simulate concurrent users
    void simulateConcurrentUsers(BTree<int, Junction>& btree, 
                                HashTable<int, Junction>& htable,
                                int userCount, int queriesPerUser = 100) {
        std::cout << "\n🧪 STRESS TEST: " << userCount << " Concurrent Users\n";
        std::cout << "=================================\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        std::atomic<int> successCount(0);
        
        for (int i = 0; i < userCount; i++) {
            threads.emplace_back([&, i]() {
                std::mt19937 localRng(i);
                std::uniform_int_distribution<> dist(1, 20000);
                
                for (int q = 0; q < queriesPerUser; q++) {
                    int id = dist(localRng);
                    Junction j;
                    
                    // Alternate between B-Tree and Hash Table
                    if (q % 2 == 0) {
                        if (btree.search(id, &j)) {
                            successCount++;
                        }
                    } else {
                        if (htable.search(id, &j)) {
                            successCount++;
                        }
                    }
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        int totalQueries = userCount * queriesPerUser;
        double queriesPerSec = (totalQueries * 1000.0) / duration.count();
        
        std::cout << "✅ Results:\n";
        std::cout << "  Total Queries: " << totalQueries << "\n";
        std::cout << "  Successful: " << successCount << "\n";
        std::cout << "  Duration: " << duration.count() << " ms\n";
        std::cout << "  Throughput: " << queriesPerSec << " queries/sec\n";
        std::cout << "  Avg Latency: " << (double)duration.count() / totalQueries << " ms\n";
    }
    
    // Load massive dataset
    void loadMassiveData(BTree<int, Junction>& btree, 
                        HashTable<int, Junction>& htable,
                        int count) {
        std::cout << "\n📦 LOADING " << count << " TEST JUNCTIONS\n";
        std::cout << "=================================\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::uniform_real_distribution<> latDist(24.0, 37.0);  // Pakistan lat range
        std::uniform_real_distribution<> lngDist(61.0, 77.0);  // Pakistan lng range
        
        for (int i = 20000; i < 20000 + count; i++) {
            Junction j;
            j.id = i;
            j.name = "Test Junction " + std::to_string(i);
            j.latitude = latDist(rng);
            j.longitude = lngDist(rng);
            j.city = "Test City";
            j.area = "Test Area";
            
            btree.insert(i, j);
            htable.insert(i, j);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "✅ Loaded " << count << " junctions in " 
                  << duration.count() << " ms\n";
        std::cout << "  B-Tree Height: " << btree.getHeight() << "\n";
        std::cout << "  Hash Load Factor: " << htable.getLoadFactor() << "\n";
        std::cout << "  Insertion Rate: " << (count * 1000.0) / duration.count() 
                  << " ops/sec\n";
    }
};

// ============ FEATURE 5: METRICS DASHBOARD ============

class MetricsDashboard {
public:
    static void displayMetrics(const BTree<int, Junction>& btree,
                              const HashTable<int, Junction>& htable) {
        std::cout << "\n📊 LIVE METRICS DASHBOARD\n";
        std::cout << "=================================================\n";
        
        auto btreeMetrics = btree.getMetrics();
        auto htableMetrics = htable.getMetrics();
        
        std::cout << "\n🌲 B-TREE METRICS:\n";
        std::cout << "  Height: " << btreeMetrics.height << " (O(log n) guarantee)\n";
        std::cout << "  Nodes: " << btreeMetrics.nodeCount << "\n";
        std::cout << "  Elements: " << btreeMetrics.elementCount << "\n";
        std::cout << "  Avg Keys/Node: " << btreeMetrics.avgKeysPerNode << "\n";
        std::cout << "  Memory: " << (btreeMetrics.memoryBytes / 1024.0) << " KB\n";
        std::cout << "  Search Ops: " << btreeMetrics.searchOps << "\n";
        
        std::cout << "\n# HASH TABLE METRICS:\n";
        std::cout << "  Elements: " << htableMetrics.elementCount << "\n";
        std::cout << "  Buckets: " << htableMetrics.bucketCount << "\n";
        std::cout << "  Load Factor: " << htableMetrics.loadFactor 
                  << " (target: 0.75)\n";
        std::cout << "  Longest Chain: " << htableMetrics.longestChain << "\n";
        std::cout << "  Avg Chain: " << htableMetrics.avgChainLength << "\n";
        std::cout << "  Collisions: " << htableMetrics.collisions << "\n";
        std::cout << "  Rehashes: " << htableMetrics.rehashes << "\n";
        std::cout << "  Memory: " << (htableMetrics.memoryUsageBytes / 1024.0) << " KB\n";
        std::cout << "  Search Ops: " << htableMetrics.searchOps << "\n";
        
        std::cout << "\n⚡ EFFICIENCY ANALYSIS:\n";
        std::cout << "  B-Tree Lookup: O(log " << btreeMetrics.elementCount 
                  << ") = " << btreeMetrics.height << " comparisons max\n";
        std::cout << "  Hash Lookup: O(1) average = " 
                  << htableMetrics.avgChainLength << " comparisons avg\n";
        
        double btreeFillRatio = btreeMetrics.avgKeysPerNode / 
                               (2 * 3 - 1); // assuming degree 3
        std::cout << "  B-Tree Fill: " << (btreeFillRatio * 100) << "%\n";
        std::cout << "  Hash Fill: " << (htableMetrics.loadFactor * 100) << "%\n";
        
        std::cout << "=================================================\n";
    }
};

// ============ FEATURE 6: PERSISTENCE ENGINE ============

class PersistenceEngine {
public:
    // Save B-Tree to disk
    template<typename K, typename V>
    static bool saveBTree(const BTree<K, V>& tree, const std::string& filename) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        auto data = tree.getAll();
        size_t count = data.size();
        
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        for (const auto& [key, value] : data) {
            // Serialize key and value (Junction in this case)
            if constexpr (std::is_same_v<V, Junction>) {
                file.write(reinterpret_cast<const char*>(&key), sizeof(key));
                file.write(reinterpret_cast<const char*>(&value.id), sizeof(value.id));
                
                size_t nameLen = value.name.length();
                file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
                file.write(value.name.c_str(), nameLen);
                
                file.write(reinterpret_cast<const char*>(&value.latitude), sizeof(value.latitude));
                file.write(reinterpret_cast<const char*>(&value.longitude), sizeof(value.longitude));
                
                size_t cityLen = value.city.length();
                file.write(reinterpret_cast<const char*>(&cityLen), sizeof(cityLen));
                file.write(value.city.c_str(), cityLen);
                
                size_t areaLen = value.area.length();
                file.write(reinterpret_cast<const char*>(&areaLen), sizeof(areaLen));
                file.write(value.area.c_str(), areaLen);
            }
        }
        
        file.close();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "💾 SAVED: " << count << " records to disk in " 
                  << duration.count() << " ms\n";
        
        return true;
    }
    
    // Load B-Tree from disk
    template<typename K, typename V>
    static bool loadBTree(BTree<K, V>& tree, const std::string& filename) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        size_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        for (size_t i = 0; i < count; i++) {
            K key;
            V value;
            
            if constexpr (std::is_same_v<V, Junction>) {
                file.read(reinterpret_cast<char*>(&key), sizeof(key));
                file.read(reinterpret_cast<char*>(&value.id), sizeof(value.id));
                
                size_t nameLen;
                file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
                value.name.resize(nameLen);
                file.read(&value.name[0], nameLen);
                
                file.read(reinterpret_cast<char*>(&value.latitude), sizeof(value.latitude));
                file.read(reinterpret_cast<char*>(&value.longitude), sizeof(value.longitude));
                
                size_t cityLen;
                file.read(reinterpret_cast<char*>(&cityLen), sizeof(cityLen));
                value.city.resize(cityLen);
                file.read(&value.city[0], cityLen);
                
                size_t areaLen;
                file.read(reinterpret_cast<char*>(&areaLen), sizeof(areaLen));
                value.area.resize(areaLen);
                file.read(&value.area[0], areaLen);
                
                tree.insert(key, value);
            }
        }
        
        file.close();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "📁 LOADED: " << count << " records from disk in " 
                  << duration.count() << " ms\n";
        
        return true;
    }
};

#endif // DS_SHOWCASE_H
