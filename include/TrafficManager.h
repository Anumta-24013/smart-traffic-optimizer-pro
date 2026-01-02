/**
 * Smart Traffic Route Optimizer - OSM Nominatim Version
 * 
 * Data Flow:
 * 1. Load OSM data into B-Tree (1299 junctions)
 * 2. Search B-Tree first (O(log n) - FAST)
 * 3. If not found, query Nominatim API (OpenStreetMap)
 * 4. Cache Nominatim results in B-Tree for future
 */

#ifndef TRAFFICMANAGER_H
#define TRAFFICMANAGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <thread>
#include <chrono>
#include <algorithm>
#include <windows.h>
#include <winhttp.h>
#include "BTree.h"
#include "HashTable.h"
#include "Graph.h"
#include "LRUCache.h"
#include "Models.h"
#include "SessionManager.h"

#pragma comment(lib, "winhttp.lib")

// Nominatim API Configuration (OpenStreetMap - FREE)
const std::wstring NOMINATIM_HOST = L"nominatim.openstreetmap.org";
const std::wstring USER_AGENT = L"TrafficOptimizer/1.0 (contact@example.com)";

class TrafficManager {
private:
    // Primary data structures
    BTree<std::string, int> junctionNameIndex;      // Name -> Junction ID
    BTree<std::string, std::vector<int>> cityIndex; // City -> List of Junction IDs
    BTree<std::string, Junction> nominatimCache;    // Nominatim search cache
    HashTable<int, Junction> junctionTable;          // ID -> Junction (O(1) lookup)
    HashTable<int, Road> roadTable;                  // Road ID -> Road
    Graph roadNetwork;                               // Road network graph
    LRUCache<std::string, RouteResult> routeCache;   // Cache for routes
    
    // User management
    BTree<std::string, User> userTree;
    HashTable<std::string, Session> sessionTable;
    SessionManager sessionManager;

    // Thread safety
    mutable std::mutex dataMutex;
    mutable std::mutex cacheMutex;
    mutable std::mutex nominatimMutex;
    
    int nextNominatimJunctionId;  // Start Nominatim IDs at 10000

    // Generate cache key for route
    std::string generateCacheKey(int source, int dest, bool useTime) const {
        return std::to_string(source) + "_" + std::to_string(dest) + 
               "_" + (useTime ? "time" : "dist");
    }

    // ==================== WINHTTP HELPER FUNCTIONS ====================

    // Convert string to wstring
    std::wstring stringToWString(const std::string& str) {
        if (str.empty()) return std::wstring();
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
        std::wstring wstr(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
        return wstr;
    }

    // URL Encode
    std::string urlEncode(const std::string& str) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (char c : str) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            } else if (c == ' ') {
                escaped << '+';
            } else {
                escaped << '%' << std::setw(2) << int((unsigned char)c);
            }
        }
        return escaped.str();
    }

    // Make HTTP Request using WinHTTP
    std::string makeHttpRequest(const std::string& path) {
        std::string response;
        HINTERNET hSession = NULL;
        HINTERNET hConnect = NULL;
        HINTERNET hRequest = NULL;

        try {
            // Initialize WinHTTP
            hSession = WinHttpOpen(USER_AGENT.c_str(),
                                   WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                   WINHTTP_NO_PROXY_NAME,
                                   WINHTTP_NO_PROXY_BYPASS, 0);
            
            if (!hSession) {
                std::cerr << "❌ WinHTTP session failed" << std::endl;
                return "";
            }

            // Connect to server
            hConnect = WinHttpConnect(hSession, NOMINATIM_HOST.c_str(),
                                      INTERNET_DEFAULT_HTTPS_PORT, 0);
            
            if (!hConnect) {
                std::cerr << "❌ WinHTTP connect failed" << std::endl;
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Create request
            std::wstring wPath = stringToWString(path);
            hRequest = WinHttpOpenRequest(hConnect, L"GET", wPath.c_str(),
                                          NULL, WINHTTP_NO_REFERER,
                                          WINHTTP_DEFAULT_ACCEPT_TYPES,
                                          WINHTTP_FLAG_SECURE);
            
            if (!hRequest) {
                std::cerr << "❌ WinHTTP request failed" << std::endl;
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Send request
            if (!WinHttpSendRequest(hRequest,
                                    WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                    WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
                std::cerr << "❌ WinHTTP send failed" << std::endl;
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Receive response
            if (!WinHttpReceiveResponse(hRequest, NULL)) {
                std::cerr << "❌ WinHTTP receive failed" << std::endl;
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }

            // Read data
            DWORD dwSize = 0;
            DWORD dwDownloaded = 0;
            char* pszOutBuffer;

            do {
                dwSize = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                    break;
                }

                pszOutBuffer = new char[dwSize + 1];
                ZeroMemory(pszOutBuffer, dwSize + 1);

                if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
                    delete[] pszOutBuffer;
                    break;
                }

                response.append(pszOutBuffer, dwDownloaded);
                delete[] pszOutBuffer;

            } while (dwSize > 0);

            // Cleanup
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);

        } catch (...) {
            if (hRequest) WinHttpCloseHandle(hRequest);
            if (hConnect) WinHttpCloseHandle(hConnect);
            if (hSession) WinHttpCloseHandle(hSession);
            return "";
        }

        return response;
    }

    // Simple JSON Parser for Nominatim response
    Junction* parseNominatimResponse(const std::string& jsonResponse, const std::string& searchQuery) {
        if (jsonResponse.empty() || jsonResponse[0] != '[') {
            return nullptr;
        }

        // Find first result
        size_t latPos = jsonResponse.find("\"lat\":");
        size_t lonPos = jsonResponse.find("\"lon\":");
        size_t namePos = jsonResponse.find("\"display_name\":");

        if (latPos == std::string::npos || lonPos == std::string::npos) {
            return nullptr;
        }

        // Extract latitude
        latPos += 7; // Skip "lat\":"
        size_t latEnd = jsonResponse.find("\"", latPos);
        std::string latStr = jsonResponse.substr(latPos, latEnd - latPos);
        double lat = std::stod(latStr);

        // Extract longitude
        lonPos += 7; // Skip "lon\":"
        size_t lonEnd = jsonResponse.find("\"", lonPos);
        std::string lonStr = jsonResponse.substr(lonPos, lonEnd - lonPos);
        double lon = std::stod(lonStr);

        // Extract display name
        std::string displayName = searchQuery; // Default to search query
        if (namePos != std::string::npos) {
            namePos += 16; // Skip "display_name\":"
            size_t nameEnd = jsonResponse.find("\"", namePos);
            displayName = jsonResponse.substr(namePos, nameEnd - namePos);
        }

        // Extract city from display name
        std::string city = "Unknown";
        std::string area = "Central";
        
        if (displayName.find("Lahore") != std::string::npos) city = "Lahore";
        else if (displayName.find("Karachi") != std::string::npos) city = "Karachi";
        else if (displayName.find("Islamabad") != std::string::npos) city = "Islamabad";
        else if (displayName.find("Rawalpindi") != std::string::npos) city = "Rawalpindi";
        else if (displayName.find("Faisalabad") != std::string::npos) city = "Faisalabad";
        else if (displayName.find("Multan") != std::string::npos) city = "Multan";
        
        // Try to extract area
        std::vector<std::string> areas = {"Gulberg", "Defence", "Model Town", 
                                         "Johar Town", "Garden Town", "Township",
                                         "Anarkali", "Cantt", "Saddar", "PECHS"};
        for (const auto& a : areas) {
            if (displayName.find(a) != std::string::npos) {
                area = a;
                break;
            }
        }

        Junction* newJunction = new Junction(
            nextNominatimJunctionId++,
            searchQuery, // Use search query as name
            lat,
            lon,
            city,
            area
        );

        return newJunction;
    }


public:
    TrafficManager(size_t cacheSize = 100) 
        : routeCache(cacheSize), nextNominatimJunctionId(10000) {}

    // ==================== SMART SEARCH ====================

    // In TrafficManager.h, update the smartSearch function:

    std::vector<Junction> smartSearch(const std::string& query, const std::string& city = "") {
        std::vector<Junction> results;
    
        // Step 1: Try fuzzy search first (NEW - BETTER!)
        std::cout << "🔍 Step 1: Fuzzy searching B-Tree..." << std::endl;
        results = fuzzySearchJunctions(query, 0.5);  // 50% similarity threshold
    
        if (!results.empty()) {
            std::cout << "✅ Found " << results.size() << " results in B-Tree (fuzzy)" << std::endl;
            return results;
        }

        // Step 2: Query Nominatim if nothing found
        std::cout << "🌍 Step 2: Querying Nominatim..." << std::endl;
    
        // Clean query - remove city if already present
        std::string actualQuery = query;
        std::string actualCity = city;
    
        std::string lowerQuery = query;
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        std::string lowerCity = city;
        std::transform(lowerCity.begin(), lowerCity.end(), lowerCity.begin(), ::tolower);
    
        if (!city.empty() && lowerQuery.find(lowerCity) != std::string::npos) {
            actualCity = "";  // City already in query
        }
    
        Junction* nominatimResult = searchNominatim(actualQuery, actualCity);
    
        if (nominatimResult) {
            addJunction(*nominatimResult);
            results.push_back(*nominatimResult);
            delete nominatimResult;
            std::cout << "✅ Added from OpenStreetMap" << std::endl;
        } else {
            std::cout << "❌ No results found" << std::endl;
        }

        return results;
    }

    // ==================== Original Junction Management ====================

    void addJunction(const Junction& junction) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        junctionTable.insert(junction.id, junction);
        junctionNameIndex.insert(junction.name, junction.id);
        
        std::vector<int> cityJunctions;
        if (cityIndex.search(junction.city, &cityJunctions)) {
            cityJunctions.push_back(junction.id);
            cityIndex.insert(junction.city, cityJunctions);
        } else {
            cityIndex.insert(junction.city, {junction.id});
        }
        
        roadNetwork.addVertex(junction.id);
    }

    bool getJunction(int id, Junction* result) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        return junctionTable.search(id, result);
    }

    bool getJunctionByName(const std::string& name, Junction* result) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        int id;
        if (junctionNameIndex.search(name, &id)) {
            return junctionTable.search(id, result);
        }
        return false;
    }

    std::vector<Junction> getAllJunctions() const {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<Junction> result;
        for (const auto& pair : junctionTable.getAll()) {
            result.push_back(pair.second);
        }
        return result;
    }

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

    std::vector<Junction> searchJunctions(const std::string& query) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<Junction> result;
    
        std::string lowerQuery = query;
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
        // Remove common words
        std::vector<std::string> stopWords = {"lahore", "karachi", "islamabad", "pakistan", "chowk", "road"};
        for (const auto& word : stopWords) {
            size_t pos = lowerQuery.find(word);
            if (pos != std::string::npos) {
                lowerQuery.erase(pos, word.length());
            }
        }
    
        // Trim spaces
        lowerQuery.erase(0, lowerQuery.find_first_not_of(" "));
        lowerQuery.erase(lowerQuery.find_last_not_of(" ") + 1);
    
        for (const auto& pair : junctionTable.getAll()) {
            std::string lowerName = pair.second.name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
            // NEW: Check if EITHER query contains name OR name contains query
            if (lowerName.find(lowerQuery) != std::string::npos || 
                lowerQuery.find(lowerName) != std::string::npos) {
                result.push_back(pair.second);
            }
        }
        return result;
    }
    
    // Normalize string: lowercase, remove extra spaces, remove common words
    std::string normalizeString(const std::string& str) const {
        std::string result = str;
        
        // Convert to lowercase
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        
        // Remove extra spaces
        result.erase(std::unique(result.begin(), result.end(), 
            [](char a, char b) { return a == ' ' && b == ' '; }), result.end());
        
        // Trim leading/trailing spaces
        size_t start = result.find_first_not_of(" ");
        size_t end = result.find_last_not_of(" ");
        if (start != std::string::npos && end != std::string::npos) {
            result = result.substr(start, end - start + 1);
        }
        
        // Remove common suffixes
        std::vector<std::string> suffixes = {" chowk", " road", " lahore", " karachi", 
                                             " islamabad", " pakistan", " junction"};
        for (const auto& suffix : suffixes) {
            size_t pos = result.find(suffix);
            if (pos != std::string::npos) {
                result.erase(pos, suffix.length());
            }
        }
        
        return result;
    }
    
    // Fuzzy search with similarity threshold
    std::vector<Junction> fuzzySearchJunctions(const std::string& query, double threshold = 0.6) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<std::pair<Junction, double>> scoredResults;
        
        std::string normalizedQuery = normalizeString(query);
        
        for (const auto& pair : junctionTable.getAll()) {
            std::string normalizedName = normalizeString(pair.second.name);
            
            // Calculate similarity
            double similarity = calculateSimilarity(normalizedQuery, normalizedName);
            
            // Also check if query is a substring (partial match bonus)
            if (normalizedName.find(normalizedQuery) != std::string::npos) {
                similarity = std::max(similarity, 0.8);  // Boost partial matches
            }
            
            // Check if name is a substring of query (reverse partial match)
            if (normalizedQuery.find(normalizedName) != std::string::npos) {
                similarity = std::max(similarity, 0.85);
            }
            
            if (similarity >= threshold) {
                scoredResults.push_back({pair.second, similarity});
            }
        }
        
        // Sort by similarity score (descending)
        std::sort(scoredResults.begin(), scoredResults.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // Return top results
        std::vector<Junction> result;
        for (const auto& scored : scoredResults) {
            result.push_back(scored.first);
            if (result.size() >= 10) break;  // Limit to top 10
        }
        
        return result;
    }
    
    // Smart search with multiple strategies
    std::vector<Junction> intelligentSearch(const std::string& query) const {
        // Strategy 1: Exact substring match
        auto exactResults = searchJunctions(query);
        if (!exactResults.empty()) return exactResults;
        
        // Strategy 2: Fuzzy match (70% threshold)
        auto fuzzyResults = fuzzySearchJunctions(query, 0.7);
        if (!fuzzyResults.empty()) return fuzzyResults;
        
        // Strategy 3: More lenient fuzzy match (50% threshold)
        return fuzzySearchJunctions(query, 0.5);
    }

    // ==================== FUZZY SEARCH ====================
    // Calculate Levenshtein distance (edit distance)
    int levenshteinDistance(const std::string& s1, const std::string& s2) const {
        const size_t len1 = s1.size(), len2 = s2.size();
        std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));
    
        for (size_t i = 0; i <= len1; ++i) d[i][0] = i;
        for (size_t j = 0; j <= len2; ++j) d[0][j] = j;
    
        for (size_t i = 1; i <= len1; ++i) {
            for (size_t j = 1; j <= len2; ++j) {
                int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
                d[i][j] = std::min({
                    d[i-1][j] + 1,      // deletion
                    d[i][j-1] + 1,      // insertion
                    d[i-1][j-1] + cost  // substitution
                });
            }
        }
        return d[len1][len2];
    }

    // Calculate similarity percentage (0.0 to 1.0)
    double calculateSimilarity(const std::string& s1, const std::string& s2) const {
        if (s1.empty() || s2.empty()) return 0.0;
    
        int distance = levenshteinDistance(s1, s2);
        int maxLen = std::max(s1.length(), s2.length());
    
        return 1.0 - (static_cast<double>(distance) / maxLen);
    }

    // Convert to lowercase
    std::string toLowerCase(const std::string& str) const {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    // Remove common words like "Lahore", "Chowk", etc.
    std::string cleanQuery(const std::string& query) const {
        std::string cleaned = query;
    
        // Remove city names
        std::vector<std::string> stopWords = {
            "lahore", "karachi", "islamabad", "rawalpindi", "faisalabad",
            "pakistan", "chowk", "road", "street", "plaza", "market"
        };
    
        std::string lower = toLowerCase(cleaned);
        for (const auto& word : stopWords) {
            size_t pos = lower.find(word);
            if (pos != std::string::npos) {
                cleaned.erase(pos, word.length());
                lower.erase(pos, word.length());
            }
        }
    
        // Remove extra spaces
        cleaned.erase(0, cleaned.find_first_not_of(" \t\n\r"));
        cleaned.erase(cleaned.find_last_not_of(" \t\n\r") + 1);
    
        return cleaned;
    }

    // ==================== Road Management ====================

    void addRoad(const Road& road) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        roadTable.insert(road.id, road);
        
        double trafficMult = getTrafficMultiplier(road.trafficLevel);
        if (road.isTwoWay) {
            roadNetwork.addUndirectedEdge(road.sourceJunction, road.destJunction,
                                          road.distance, road.baseTime, road.name);
        } else {
            roadNetwork.addEdge(road.sourceJunction, road.destJunction,
                               road.distance, road.baseTime, road.name);
        }
        
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

    bool getRoad(int id, Road* result) const {
        std::lock_guard<std::mutex> lock(dataMutex);
        return roadTable.search(id, result);
    }

    std::vector<Road> getAllRoads() const {
        std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<Road> result;
        for (const auto& pair : roadTable.getAll()) {
            result.push_back(pair.second);
        }
        return result;
    }

    bool updateTrafficLevel(int roadId, TrafficLevel level) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        Road road;
        if (!roadTable.search(roadId, &road)) {
            return false;
        }
        
        road.trafficLevel = level;
        roadTable.insert(roadId, road);
        
        double multiplier = getTrafficMultiplier(level);
        if (road.isTwoWay) {
            roadNetwork.updateTrafficBidirectional(road.sourceJunction, 
                                                   road.destJunction, multiplier);
        } else {
            roadNetwork.updateTraffic(road.sourceJunction, 
                                      road.destJunction, multiplier);
        }
        
        invalidateCache();
        return true;
    }

    // ==================== Route Finding ====================

    // In TrafficManager.h, modify the findRoute function:

    // ==================== FIXED Route Finding ====================

    RouteResult findRoute(int sourceId, int destId, bool useTime = true) {
        std::string cacheKey = generateCacheKey(sourceId, destId, useTime);
    
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            RouteResult cached;
            if (routeCache.get(cacheKey, &cached)) {
                return cached;
            }
        }
    
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
        
            // Populate complete junction details
            for (int junctionId : pathResult.path) {
                Junction j;
                if (junctionTable.search(junctionId, &j)) {
                    result.junctions.push_back(j);
                }
            }
        
            // Build traffic segments for visualization
            for (size_t i = 0; i < pathResult.path.size() - 1; ++i) {
                int fromId = pathResult.path[i];
                int toId = pathResult.path[i + 1];
            
                const Edge* edge = roadNetwork.getEdge(fromId, toId);
                if (edge) {
                    // Determine traffic level from multiplier
                    TrafficLevel level = TrafficLevel::NORMAL;
                    if (edge->trafficMultiplier <= 0.8) {
                        level = TrafficLevel::LOW;
                    } else if (edge->trafficMultiplier <= 1.0) {
                        level = TrafficLevel::NORMAL;
                    } else if (edge->trafficMultiplier <= 1.5) {
                        level = TrafficLevel::HEAVY;
                    } else {
                        level = TrafficLevel::SEVERE;
                    }
                
                    TrafficSegment segment(
                        fromId,
                        toId,
                        edge->roadName,
                        edge->distance,
                        edge->getActualTime(),
                        level
                    );
                    result.trafficSegments.push_back(segment);
                }
            }
        }
    
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            routeCache.put(cacheKey, result);
        }
    
        return result;
    }

    RouteResult findRouteByName(const std::string& sourceName, 
                                const std::string& destName,
                                bool useTime = true) {
        int sourceId, destId;
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (!junctionNameIndex.search(sourceName, &sourceId) ||
                !junctionNameIndex.search(destName, &destId)) {
                return RouteResult();
            }
        }
        return findRoute(sourceId, destId, useTime);
    }


    // RouteResult findRoute(int sourceId, int destId, bool useTime = true) {
    //     std::string cacheKey = generateCacheKey(sourceId, destId, useTime);
    //     {
    //         std::lock_guard<std::mutex> lock(cacheMutex);
    //         RouteResult cached;
    //         if (routeCache.get(cacheKey, &cached)) {
    //             return cached;
    //         }
    //     }
    
    //     PathResult pathResult;
    //     {
    //         std::lock_guard<std::mutex> lock(dataMutex);
    //         pathResult = roadNetwork.dijkstra(sourceId, destId, useTime);
    //     }
    
    //     RouteResult result;
    //     result.found = pathResult.found;
    //     result.totalDistance = pathResult.totalDistance;
    //     result.totalTime = pathResult.totalTime;
    
    //     if (pathResult.found) {
    //         std::lock_guard<std::mutex> lock(dataMutex);
    //         for (int junctionId : pathResult.path) {
    //             Junction j;
    //             if (junctionTable.search(junctionId, &j)) {
    //                 result.junctions.push_back(j);
                
    //                 // Also add roads between junctions
    //                 if (!result.junctions.empty()) {
    //                     int prevId = result.junctions[result.junctions.size() - 2].id;
    //                     Edge* edge = roadNetwork.getEdge(prevId, junctionId);
    //                     if (edge) {
    //                         Road road;
    //                         // Create a road object for this segment
    //                         road.id = result.roads.size() + 1;
    //                         road.sourceJunction = prevId;
    //                         road.destJunction = junctionId;
    //                         road.distance = edge->distance;
    //                         road.baseTime = edge->baseTime;
    //                         road.trafficMultiplier = edge->trafficMultiplier;
    //                         road.name = edge->roadName;
    //                         road.isTwoWay = true; // Assuming bidirectional
    //                         result.roads.push_back(road);
    //                     }
    //                 }
    //             }
    //         }
    //     }
    
    //     {
    //         std::lock_guard<std::mutex> lock(cacheMutex);
    //         routeCache.put(cacheKey, result);
    //     }
        
    //     return result;
    // }

    // RouteResult findRouteByName(const std::string& sourceName, 
    //                             const std::string& destName,
    //                             bool useTime = true) {
    //     int sourceId, destId;
    //     {
    //         std::lock_guard<std::mutex> lock(dataMutex);
    //         if (!junctionNameIndex.search(sourceName, &sourceId) ||
    //             !junctionNameIndex.search(destName, &destId)) {
    //             return RouteResult();
    //         }
    //     }
    //     return findRoute(sourceId, destId, useTime);
    // }

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
        std::cout << "  - OSM Junctions: " << (getJunctionCount() - (nextNominatimJunctionId - 10000)) << "\n";
        std::cout << "  - Nominatim Results: " << (nextNominatimJunctionId - 10000) << "\n";
        std::cout << "Roads: " << getRoadCount() << "\n";
        std::cout << "Graph Vertices: " << roadNetwork.getNumVertices() << "\n";
        std::cout << "Graph Edges: " << roadNetwork.getNumEdges() << "\n";
        std::cout << "Cache Hit Rate: " << getCacheHitRate() << "%\n";
        std::cout << "==================================\n";
    }

    // ==================== Data Persistence ====================

    bool loadJunctionsFromJson(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << "\n";
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        size_t pos = 0;
        while ((pos = content.find("\"id\":", pos)) != std::string::npos) {
            Junction j;
            
            pos += 5;
            j.id = std::stoi(content.substr(pos, content.find(",", pos) - pos));
            
            pos = content.find("\"name\":", pos) + 8;
            size_t nameEnd = content.find("\"", pos);
            j.name = content.substr(pos, nameEnd - pos);
            
            pos = content.find("\"latitude\":", pos) + 11;
            j.latitude = std::stod(content.substr(pos, content.find(",", pos) - pos));
            
            pos = content.find("\"longitude\":", pos) + 12;
            j.longitude = std::stod(content.substr(pos, content.find(",", pos) - pos));
            
            pos = content.find("\"city\":", pos) + 8;
            size_t cityEnd = content.find("\"", pos);
            j.city = content.substr(pos, cityEnd - pos);
            
            pos = content.find("\"area\":", pos) + 8;
            size_t areaEnd = content.find("\"", pos);
            j.area = content.substr(pos, areaEnd - pos);
            
            addJunction(j);
        }
        
        return true;
    }

    // ==================== User & Session Management ====================
    
    bool registerUser(const std::string& username, const std::string& email,
                     const std::string& passwordHash) {
        std::lock_guard<std::mutex> lock(dataMutex);
        
        User existing;
        if (userTree.search(username, &existing)) {
            return false;
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

    std::string createUserSession(int userId, const std::string& username, 
                                   const std::string& ipAddress = "") {
        return sessionManager.createSession(userId, username, ipAddress);
    }
    
    bool validateToken(const std::string& token, int* userId = nullptr, 
                      std::string* username = nullptr) const {
        return sessionManager.validateToken(token, userId, username);
    }
    
    bool logoutUser(const std::string& token) {
        return sessionManager.invalidateSession(token);
    }
    
    size_t getActiveUserCount() const {
        return sessionManager.getActiveSessionCount();
    }
    
    int cleanExpiredSessions() {
        return sessionManager.cleanExpiredSessions();
    }
    
    std::vector<std::string> getActiveUsers() const {
        return sessionManager.getActiveUsers();
    }

public:

    Junction* searchNominatim(const std::string& query, const std::string& city = "") {
        std::lock_guard<std::mutex> lock(nominatimMutex);
    
        // Check cache first
        Junction cached;
        std::string cacheKey = query + "_" + city;
        if (nominatimCache.search(cacheKey, &cached)) {
            std::cout << "✅ Found in Nominatim cache: " << cached.name << std::endl;
            return new Junction(cached);
        }

        // Build SMART query
        std::string searchQuery = query;
    
        // Remove common words that confuse Nominatim
        std::vector<std::string> removeWords = {"lahore", "karachi", "islamabad", "pakistan"};
        std::string lowerQuery = query;
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
        for (const auto& word : removeWords) {
            size_t pos = lowerQuery.find(word);
            if (pos != std::string::npos) {
                searchQuery.erase(pos, word.length());
            }
        }
    
        // Trim spaces
        searchQuery.erase(0, searchQuery.find_first_not_of(" "));
        searchQuery.erase(searchQuery.find_last_not_of(" ") + 1);
    
        // Add city context
        if (!city.empty()) {
            searchQuery += " " + city;
        }
        searchQuery += " Pakistan";
    
        std::string encodedQuery = urlEncode(searchQuery);
    
        // Build URL path
        std::string path = "/search?q=" + encodedQuery +
                          "&format=json&limit=1&countrycodes=pk";

        std::cout << "🌍 Searching Nominatim for: " << searchQuery << std::endl;
    
        // Make request
        std::string response = makeHttpRequest(path);
        if (response.empty()) {
            return nullptr;
        }

        // Parse response
        Junction* newJunction = parseNominatimResponse(response, query);
    
        if (newJunction) {
            nominatimCache.insert(cacheKey, *newJunction);
            std::cout << "✅ Found via Nominatim: " << newJunction->name 
                      << " at (" << newJunction->latitude << ", " << newJunction->longitude << ")" << std::endl;
        }

        // Rate limiting
        std::this_thread::sleep_for(std::chrono::seconds(1));

        return newJunction;
    }

};

#endif // TRAFFICMANAGER_H
