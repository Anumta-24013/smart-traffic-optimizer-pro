/**
 * Smart Traffic Route Optimizer
 * Traffic Manager - Core System Logic
 * 
 * Combines all data structures:
 * - B-Tree for junction indexing
 * - Hash Table for O(1) lookup
 * - Graph for road network
 * - LRU Cache for route caching
 */

#ifndef TRAFFICMANAGER_H
#define TRAFFICMANAGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <thread>
#include "BTree.h"
#include "HashTable.h"
#include "Graph.h"
#include "LRUCache.h"
#include "Models.h"

class TrafficManager {
private:
    // Primary data structures
    BTree<std::string, int> junctionNameIndex;      // Name -> Junction ID
    BTree<std::string, std::vector<int>> cityIndex; // City -> List of Junction IDs
    HashTable<int, Junction> junctionTable;          // ID -> Junction (O(1) lookup)
    HashTable<int, Road> roadTable;                  // Road ID -> Road
    Graph roadNetwork;                               // Road network graph
    LRUCache<std::string, RouteResult> routeCache;   // Cache for routes
    
    // User management
    BTree<std::string, User> userTree;               // Username -> User
    HashTable<std::string, Session> sessionTable;     // Token -> Session
    
    // Thread safety
    mutable std::mutex dataMutex;
    mutable std::mutex cacheMutex;

    // Generate cache key for route
    std::string generateCacheKey(int source, int dest, bool useTime) const {
        return std::to_string(source) + "_" + std::to_string(dest) + 
               "_" + (useTime ? "time" : "dist");
    }

public:
    TrafficManager(size_t cacheSize = 100) : routeCache(cacheSize) {}

    // ==================== Junction Management ====================

    // Add a junction
    void addJunction(const Junction& junction) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        // Add to hash table for O(1) lookup
        junctionTable.insert(junction.id, junction);
        
        // Add to B-Tree for name-based search
        junctionNameIndex.insert(junction.name, junction.id);
        
        // Add to city index
        std::vector<int> cityJunctions;
        if (cityIndex.search(junction.city, &cityJunctions)) {
            cityJunctions.push_back(junction.id);
            cityIndex.insert(junction.city, cityJunctions);
        } else {
            cityIndex.insert(junction.city, {junction.id});
        }
        
        // Add vertex to graph
        roadNetwork.addVertex(junction.id);
    }

    // Get junction by ID - O(1)
    bool getJunction(int id, Junction* result) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        return junctionTable.search(id, result);
    }

    // Get junction by name - O(log n)
    bool getJunctionByName(const std::string& name, Junction* result) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        int id;
        if (junctionNameIndex.search(name, &id)) {
            return junctionTable.search(id, result);
        }
        return false;
    }

    // Get all junctions
    std::vector<Junction> getAllJunctions() const {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<Junction> result;
        for (const auto& pair : junctionTable.getAll()) {
            result.push_back(pair.second);
        }
        return result;
    }

    // Get junctions by city
    std::vector<Junction> getJunctionsByCity(const std::string& city) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<Junction> result;
        std::vector<int> ids;
        if (cityIndex.search(city, &ids)) {
            for (int id : ids) {
                Junction j;
                if (junctionTable.search(id, &j)) {
                    result.push_back(j);
                }
            }
        }
        return result;
    }

    // Search junctions by partial name
    std::vector<Junction> searchJunctions(const std::string& query) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<Junction> result;
        
        // Convert query to lowercase for case-insensitive search
        std::string lowerQuery = query;
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        
        for (const auto& pair : junctionTable.getAll()) {
            std::string lowerName = pair.second.name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            
            if (lowerName.find(lowerQuery) != std::string::npos) {
                result.push_back(pair.second);
            }
        }
        return result;
    }

    // ==================== Road Management ====================

    // Add a road
    void addRoad(const Road& road) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        // Add to hash table
        roadTable.insert(road.id, road);
        
        // Add to graph
        double trafficMult = getTrafficMultiplier(road.trafficLevel);
        if (road.isTwoWay) {
            roadNetwork.addUndirectedEdge(road.sourceJunction, road.destJunction,
                                          road.distance, road.baseTime, road.name);
        } else {
            roadNetwork.addEdge(road.sourceJunction, road.destJunction,
                               road.distance, road.baseTime, road.name);
        }
        
        // Update junction connections
        Junction source, dest;
        if (junctionTable.search(road.sourceJunction, &source)) {
            source.connectedJunctions.push_back(road.destJunction);
            junctionTable.insert(road.sourceJunction, source);
        }
        if (road.isTwoWay && junctionTable.search(road.destJunction, &dest)) {
            dest.connectedJunctions.push_back(road.sourceJunction);
            junctionTable.insert(road.destJunction, dest);
        }
    }

    // Get road by ID
    bool getRoad(int id, Road* result) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        return roadTable.search(id, result);
    }

    // Get all roads
    std::vector<Road> getAllRoads() const {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<Road> result;
        for (const auto& pair : roadTable.getAll()) {
            result.push_back(pair.second);
        }
        return result;
    }

    // Update traffic level on a road
    bool updateTrafficLevel(int roadId, TrafficLevel level) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        Road road;
        if (!roadTable.search(roadId, &road)) {
            return false;
        }
        
        road.trafficLevel = level;
        roadTable.insert(roadId, road);
        
        // Update graph edge
        double multiplier = getTrafficMultiplier(level);
        if (road.isTwoWay) {
            roadNetwork.updateTrafficBidirectional(road.sourceJunction, 
                                                   road.destJunction, multiplier);
        } else {
            roadNetwork.updateTraffic(road.sourceJunction, 
                                      road.destJunction, multiplier);
        }
        
        // Invalidate cache for affected routes
        invalidateCache();
        
        return true;
    }

    // Update traffic between two junctions
    bool updateTrafficBetween(int sourceId, int destId, TrafficLevel level) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        // Find road between junctions
        for (auto& pair : roadTable.getAll()) {
            if ((pair.second.sourceJunction == sourceId && 
                 pair.second.destJunction == destId) ||
                (pair.second.isTwoWay && 
                 pair.second.sourceJunction == destId && 
                 pair.second.destJunction == sourceId)) {
                
                Road road = pair.second;
                road.trafficLevel = level;
                roadTable.insert(road.id, road);
                
                double multiplier = getTrafficMultiplier(level);
                roadNetwork.updateTrafficBidirectional(sourceId, destId, multiplier);
                invalidateCache();
                return true;
            }
        }
        return false;
    }

    // ==================== Route Finding ====================

    // Find shortest route (with caching)
    RouteResult findRoute(int sourceId, int destId, bool useTime = true) {
        std::string cacheKey = generateCacheKey(sourceId, destId, useTime);
        
        // Check cache first
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            RouteResult cached;
            if (routeCache.get(cacheKey, &cached)) {
                return cached;
            }
        }
        
        // Calculate route
        PathResult pathResult;
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            pathResult = roadNetwork.dijkstra(sourceId, destId, useTime);
        }
        
        RouteResult result;
        result.found = pathResult.found;
        result.totalDistance = pathResult.totalDistance;
        result.totalTime = pathResult.totalTime;
        
        if (pathResult.found) {
            std::lock_guard<std::mutex> lock(dataMutex);
            for (int junctionId : pathResult.path) {
                Junction j;
                if (junctionTable.search(junctionId, &j)) {
                    result.junctions.push_back(j);
                }
            }
        }
        
        // Cache the result
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            routeCache.put(cacheKey, result);
        }
        
        return result;
    }

    // Find route by junction names
    RouteResult findRouteByName(const std::string& sourceName, 
                                const std::string& destName,
                                bool useTime = true) {
        int sourceId, destId;
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (!junctionNameIndex.search(sourceName, &sourceId) ||
                !junctionNameIndex.search(destName, &destId)) {
                return RouteResult(); // Not found
            }
        }
        return findRoute(sourceId, destId, useTime);
    }

    // Invalidate route cache
    void invalidateCache() {
        std::lock_guard<std::mutex> lock(cacheMutex);
        routeCache.clear();
    }

    // ==================== Statistics ====================

    int getJunctionCount() const {
        std::lock_guard<std::mutex> lock(dataMutex);
        return junctionTable.size();
    }

    int getRoadCount() const {
        std::lock_guard<std::mutex> lock(dataMutex);
        return roadTable.size();
    }

    double getCacheHitRate() const {
        std::lock_guard<std::mutex> lock(cacheMutex);
        return routeCache.getHitRate();
    }

    void printStatistics() const {
        std::cout << "\n=== Traffic Manager Statistics ===\n";
        std::cout << "Junctions: " << getJunctionCount() << "\n";
        std::cout << "Roads: " << getRoadCount() << "\n";
        std::cout << "Graph Vertices: " << roadNetwork.getNumVertices() << "\n";
        std::cout << "Graph Edges: " << roadNetwork.getNumEdges() << "\n";
        std::cout << "Cache Hit Rate: " << getCacheHitRate() << "%\n";
        std::cout << "==================================\n";
    }

    // ==================== Data Persistence ====================

    // Load junctions from JSON file
    bool loadJunctionsFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << "\n";
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        // Simple JSON parsing (for production, use a proper JSON library)
        // This is a basic implementation for demo purposes
        size_t pos = 0;
        while ((pos = content.find("\"id\":", pos)) != std::string::npos) {
            Junction j;
            
            // Parse id
            pos += 5;
            j.id = std::stoi(content.substr(pos, content.find(",", pos) - pos));
            
            // Parse name
            pos = content.find("\"name\":", pos) + 8;
            size_t nameEnd = content.find("\"", pos);
            j.name = content.substr(pos, nameEnd - pos);
            
            // Parse latitude
            pos = content.find("\"latitude\":", pos) + 11;
            j.latitude = std::stod(content.substr(pos, content.find(",", pos) - pos));
            
            // Parse longitude
            pos = content.find("\"longitude\":", pos) + 12;
            j.longitude = std::stod(content.substr(pos, content.find(",", pos) - pos));
            
            // Parse city
            pos = content.find("\"city\":", pos) + 8;
            size_t cityEnd = content.find("\"", pos);
            j.city = content.substr(pos, cityEnd - pos);
            
            // Parse area
            pos = content.find("\"area\":", pos) + 8;
            size_t areaEnd = content.find("\"", pos);
            j.area = content.substr(pos, areaEnd - pos);
            
            addJunction(j);
        }
        
        return true;
    }

    // Save junctions to JSON file
    bool saveJunctionsToJson(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        auto junctions = getAllJunctions();
        file << "{\n  \"junctions\": [\n";
        for (size_t i = 0; i < junctions.size(); ++i) {
            file << "    " << junctions[i].toJson();
            if (i < junctions.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ]\n}\n";
        
        return true;
    }

    // ==================== User Management ====================

    bool registerUser(const std::string& username, const std::string& email,
                     const std::string& passwordHash) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        User existing;
        if (userTree.search(username, &existing)) {
            return false; // User already exists
        }
        
        User newUser;
        newUser.id = userTree.size() + 1;
        newUser.username = username;
        newUser.email = email;
        newUser.passwordHash = passwordHash;
        newUser.isAdmin = false;
        
        userTree.insert(username, newUser);
        return true;
    }

    bool authenticateUser(const std::string& username, 
                         const std::string& passwordHash,
                         User* user = nullptr) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        User u;
        if (!userTree.search(username, &u)) {
            return false;
        }
        
        if (u.passwordHash != passwordHash) {
            return false;
        }
        
        if (user) *user = u;
        return true;
    }
};

#endif // TRAFFICMANAGER_H
