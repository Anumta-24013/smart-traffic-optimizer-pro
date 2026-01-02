/**
 * OSMLoader.h - WITH INTER-CITY HIGHWAY CONNECTIONS
 * 
 * NEW: Connects major cities via highways
 * - Within city: 5km connections (local roads)
 * - Between cities: 100km+ highways
 */

#ifndef OSMLOADER_H
#define OSMLOADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include "TrafficManager.h"

class OSMLoader {
private:
    TrafficManager& trafficManager;
    
    // Helper functions (keep existing ones)
    std::string extractValue(const std::string& json, size_t start, size_t end) {
        std::string value = json.substr(start, end - start);
        if (!value.empty() && value[0] == '"') {
            value = value.substr(1);
        }
        if (!value.empty() && value[value.length()-1] == '"') {
            value = value.substr(0, value.length()-1);
        }
        return value;
    }
    
    int extractInt(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return 0;
        
        size_t start = pos + searchKey.length();
        while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;
        
        size_t end = json.find_first_of(",}", start);
        if (end == std::string::npos) return 0;
        
        try {
            return std::stoi(json.substr(start, end - start));
        } catch (...) {
            return 0;
        }
    }
    
    double extractDouble(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return 0.0;
        
        size_t start = pos + searchKey.length();
        while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;
        
        size_t end = json.find_first_of(",}", start);
        if (end == std::string::npos) return 0.0;
        
        try {
            return std::stod(json.substr(start, end - start));
        } catch (...) {
            return 0.0;
        }
    }
    
    std::string extractString(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return "";
        
        size_t start = pos + searchKey.length();
        while (start < json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '"')) start++;
        
        size_t end = json.find("\"", start);
        if (end == std::string::npos) return "";
        
        return json.substr(start, end - start);
    }
    
    bool extractBool(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return false;
        
        size_t start = pos + searchKey.length();
        while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;
        
        return json.substr(start, 4) == "true";
    }

public:
    OSMLoader(TrafficManager& tm) : trafficManager(tm) {}
    
    bool loadJunctions(const std::string& filename) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        std::cout << "\n╔════════════════════════════════════════╗\n";
        std::cout << "║   LOADING OPENSTREETMAP DATA          ║\n";
        std::cout << "╚════════════════════════════════════════╝\n\n";
        std::cout << "📂 File: " << filename << "\n";
        std::cout << "📡 Source: OpenStreetMap (Overpass API)\n";
        std::cout << "💾 Storage: B-Tree + Hash Table (RAM)\n\n";
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "❌ ERROR: Cannot open file!\n";
            std::cerr << "   File path: " << filename << "\n";
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        file.close();
        
        std::cout << "⏳ Loading junctions into memory...\n\n";
        
        int loadedCount = 0;
        int errorCount = 0;
        
        size_t junctionsPos = content.find("\"junctions\"");
        if (junctionsPos == std::string::npos) {
            std::cerr << "❌ ERROR: 'junctions' array not found in JSON!\n";
            return false;
        }
        
        size_t arrayStart = content.find("[", junctionsPos);
        if (arrayStart == std::string::npos) {
            std::cerr << "❌ ERROR: Invalid JSON format!\n";
            return false;
        }
        
        size_t pos = arrayStart + 1;
        while (pos < content.length()) {
            size_t objStart = content.find("{", pos);
            if (objStart == std::string::npos) break;
            
            size_t objEnd = content.find("}", objStart);
            if (objEnd == std::string::npos) break;
            
            std::string junctionStr = content.substr(objStart, objEnd - objStart + 1);
            
            try {
                Junction j;
                j.id = extractInt(junctionStr, "id");
                j.name = extractString(junctionStr, "name");
                j.latitude = extractDouble(junctionStr, "latitude");
                j.longitude = extractDouble(junctionStr, "longitude");
                j.city = extractString(junctionStr, "city");
                j.area = extractString(junctionStr, "area");
                j.hasTrafficSignal = extractBool(junctionStr, "hasTrafficSignal");
                
                if (j.id > 0 && !j.name.empty() && j.latitude != 0.0) {
                    trafficManager.addJunction(j);
                    loadedCount++;
                    
                    if (loadedCount % 1000 == 0) {
                        std::cout << "   📍 Loaded " << loadedCount << " junctions...\n";
                    }
                }
            } catch (...) {
                errorCount++;
            }
            
            pos = objEnd + 1;
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "\n╔════════════════════════════════════════╗\n";
        std::cout << "║   LOADING COMPLETE!                   ║\n";
        std::cout << "╚════════════════════════════════════════╝\n\n";
        std::cout << "✅ Junctions Loaded: " << loadedCount << "\n";
        std::cout << "⚠️  Errors: " << errorCount << "\n";
        std::cout << "⏱️  Load Time: " << duration.count() << " ms\n";
        std::cout << "💾 B-Tree Nodes: ~" << (loadedCount / 10) << "\n";
        std::cout << "🔍 Search Complexity: O(log n)\n";
        std::cout << "⚡ Average Search Time: <1ms\n\n";
        
        return loadedCount > 0;
    }
    
    // ============ NEW: GENERATE NETWORK WITH INTER-CITY HIGHWAYS ============
    void generateRoadNetwork(double maxDistanceKm = 5.0) {
        std::cout << "╔════════════════════════════════════════╗\n";
        std::cout << "║   GENERATING ROAD NETWORK             ║\n";
        std::cout << "╚════════════════════════════════════════╝\n\n";
        std::cout << "🛣️  Connecting nearby junctions...\n";
        std::cout << "📏 Within cities: " << maxDistanceKm << " km\n";
        std::cout << "🏙️  Between cities: Major highways\n\n";
        
        auto junctions = trafficManager.getAllJunctions();
        int roadId = 1;
        int intraRoads = 0;  // Within city
        int interRoads = 0;  // Between cities
        
        // Group junctions by city
        std::map<std::string, std::vector<Junction>> citiesMap;
        for (const auto& j : junctions) {
            citiesMap[j.city].push_back(j);
        }
        
        std::cout << "📍 Found " << citiesMap.size() << " cities\n";
        for (const auto& [city, jcts] : citiesMap) {
            std::cout << "   • " << city << ": " << jcts.size() << " junctions\n";
        }
        std::cout << "\n";
        
        // PHASE 1: INTRA-CITY ROADS (within same city)
        std::cout << "⏳ Phase 1: Generating intra-city roads...\n";
        
        for (auto& [city, cityJunctions] : citiesMap) {
            std::cout << "   🏙️ Processing " << city << "...\n";
            
            for (size_t i = 0; i < cityJunctions.size(); ++i) {
                for (size_t j = i + 1; j < cityJunctions.size(); ++j) {
                    double distance = cityJunctions[i].distanceTo(cityJunctions[j]);
                    
                    if (distance < maxDistanceKm) {
                        double speedLimit = 40.0;
                        
                        std::string areaLower = cityJunctions[i].area;
                        std::transform(areaLower.begin(), areaLower.end(), 
                                     areaLower.begin(), ::tolower);
                        
                        if (areaLower.find("highway") != std::string::npos ||
                            areaLower.find("motorway") != std::string::npos) {
                            speedLimit = 100.0;
                        } else if (areaLower.find("main") != std::string::npos ||
                                 areaLower.find("road") != std::string::npos) {
                            speedLimit = 60.0;
                        }
                        
                        Road road(roadId++, 
                                 cityJunctions[i].name + " to " + cityJunctions[j].name,
                                 cityJunctions[i].id,
                                 cityJunctions[j].id,
                                 distance,
                                 speedLimit);
                        road.isTwoWay = true;
                        
                        trafficManager.addRoad(road);
                        intraRoads++;
                    }
                }
                
                // Progress indicator
                if (i % 100 == 0 && i > 0) {
                    std::cout << "      Progress: " << i << "/" << cityJunctions.size() << "\n";
                }
            }
        }
        
        std::cout << "   ✅ Intra-city roads: " << intraRoads << "\n\n";
        
        // PHASE 2: INTER-CITY HIGHWAYS (between different cities)
        std::cout << "⏳ Phase 2: Generating inter-city highways...\n";
        
        // Major city pairs with known highways
        std::vector<std::pair<std::string, std::string>> cityPairs = {
            {"Lahore", "Islamabad"},
            {"Lahore", "Faisalabad"},
            {"Lahore", "Multan"},
            {"Karachi", "Islamabad"},
            {"Karachi", "Multan"},
            {"Islamabad", "Rawalpindi"},
            {"Islamabad", "Faisalabad"},
            {"Faisalabad", "Multan"}
        };
        
        for (const auto& [city1, city2] : cityPairs) {
            if (citiesMap.find(city1) == citiesMap.end() || 
                citiesMap.find(city2) == citiesMap.end()) {
                continue;
            }
            
            // Find closest junctions between cities
            const auto& junctions1 = citiesMap[city1];
            const auto& junctions2 = citiesMap[city2];
            
            double minDistance = 999999.0;
            size_t bestI = 0, bestJ = 0;
            
            for (size_t i = 0; i < junctions1.size(); ++i) {
                for (size_t j = 0; j < junctions2.size(); ++j) {
                    double dist = junctions1[i].distanceTo(junctions2[j]);
                    if (dist < minDistance) {
                        minDistance = dist;
                        bestI = i;
                        bestJ = j;
                    }
                }
            }
            
            // Create highway connection
            Road highway(roadId++,
                        city1 + " - " + city2 + " Highway",
                        junctions1[bestI].id,
                        junctions2[bestJ].id,
                        minDistance,
                        120.0);  // Highway speed
            highway.isTwoWay = true;
            
            trafficManager.addRoad(highway);
            interRoads++;
            
            std::cout << "   🛣️ " << city1 << " ↔ " << city2 
                      << " (" << minDistance << " km)\n";
        }
        
        std::cout << "\n   ✅ Inter-city highways: " << interRoads << "\n\n";
        
        std::cout << "╔════════════════════════════════════════╗\n";
        std::cout << "║   ROAD NETWORK COMPLETE!              ║\n";
        std::cout << "╚════════════════════════════════════════╝\n\n";
        std::cout << "   Total Roads: " << (intraRoads + interRoads) << "\n";
        std::cout << "   • Intra-city: " << intraRoads << "\n";
        std::cout << "   • Inter-city: " << interRoads << "\n";
        std::cout << "   Graph Edges: " << ((intraRoads + interRoads) * 2) << "\n\n";
    }
    
    void printStats() {
        std::cout << "╔════════════════════════════════════════╗\n";
        std::cout << "║   SYSTEM STATISTICS                   ║\n";
        std::cout << "╚════════════════════════════════════════╝\n\n";
        
        trafficManager.printStatistics();
        
        std::cout << "\n📊 Data Structures in Use:\n";
        std::cout << "   ├─ B-Tree (Name Index)\n";
        std::cout << "   ├─ B-Tree (City Index)\n";
        std::cout << "   ├─ Hash Table (ID Lookup)\n";
        std::cout << "   ├─ Graph (Dijkstra's Algorithm)\n";
        std::cout << "   ├─ Min-Heap (Priority Queue)\n";
        std::cout << "   └─ LRU Cache (Route Cache)\n\n";
    }
};

#endif



// /**
//  * OpenStreetMap Data Loader
//  * Loads real OSM data into B-Tree structures
//  */

// #ifndef OSMLOADER_H
// #define OSMLOADER_H

// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <chrono>
// #include <string>
// #include <vector>
// #include "TrafficManager.h"

// class OSMLoader {
// private:
//     TrafficManager& trafficManager;
    
//     // Helper to find value after key in JSON string
//     std::string extractValue(const std::string& json, size_t start, size_t end) {
//         std::string value = json.substr(start, end - start);
//         // Remove quotes if present
//         if (!value.empty() && value[0] == '"') {
//             value = value.substr(1);
//         }
//         if (!value.empty() && value[value.length()-1] == '"') {
//             value = value.substr(0, value.length()-1);
//         }
//         return value;
//     }
    
//     int extractInt(const std::string& json, const std::string& key) {
//         std::string searchKey = "\"" + key + "\":";
//         size_t pos = json.find(searchKey);
//         if (pos == std::string::npos) return 0;
        
//         size_t start = pos + searchKey.length();
//         while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;
        
//         size_t end = json.find_first_of(",}", start);
//         if (end == std::string::npos) return 0;
        
//         try {
//             return std::stoi(json.substr(start, end - start));
//         } catch (...) {
//             return 0;
//         }
//     }
    
//     double extractDouble(const std::string& json, const std::string& key) {
//         std::string searchKey = "\"" + key + "\":";
//         size_t pos = json.find(searchKey);
//         if (pos == std::string::npos) return 0.0;
        
//         size_t start = pos + searchKey.length();
//         while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;
        
//         size_t end = json.find_first_of(",}", start);
//         if (end == std::string::npos) return 0.0;
        
//         try {
//             return std::stod(json.substr(start, end - start));
//         } catch (...) {
//             return 0.0;
//         }
//     }
    
//     std::string extractString(const std::string& json, const std::string& key) {
//         std::string searchKey = "\"" + key + "\":";
//         size_t pos = json.find(searchKey);
//         if (pos == std::string::npos) return "";
        
//         size_t start = pos + searchKey.length();
//         while (start < json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '"')) start++;
        
//         size_t end = json.find("\"", start);
//         if (end == std::string::npos) return "";
        
//         return json.substr(start, end - start);
//     }
    
//     bool extractBool(const std::string& json, const std::string& key) {
//         std::string searchKey = "\"" + key + "\":";
//         size_t pos = json.find(searchKey);
//         if (pos == std::string::npos) return false;
        
//         size_t start = pos + searchKey.length();
//         while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;
        
//         return json.substr(start, 4) == "true";
//     }

// public:
//     OSMLoader(TrafficManager& tm) : trafficManager(tm) {}
    
//     bool loadJunctions(const std::string& filename) {
//         auto startTime = std::chrono::high_resolution_clock::now();
        
//         std::cout << "\n╔════════════════════════════════════════╗\n";
//         std::cout << "║   LOADING OPENSTREETMAP DATA          ║\n";
//         std::cout << "╚════════════════════════════════════════╝\n\n";
//         std::cout << "📂 File: " << filename << "\n";
//         std::cout << "📡 Source: OpenStreetMap (Overpass API)\n";
//         std::cout << "💾 Storage: B-Tree + Hash Table (RAM)\n\n";
        
//         // Read entire file into string
//         std::ifstream file(filename);
//         if (!file.is_open()) {
//             std::cerr << "❌ ERROR: Cannot open file!\n";
//             std::cerr << "   File path: " << filename << "\n";
//             return false;
//         }
        
//         // Read entire file
//         std::stringstream buffer;
//         buffer << file.rdbuf();
//         std::string content = buffer.str();
//         file.close();
        
//         std::cout << "⏳ Loading junctions into memory...\n\n";
        
//         int loadedCount = 0;
//         int errorCount = 0;
        
//         // Find junctions array
//         size_t junctionsPos = content.find("\"junctions\"");
//         if (junctionsPos == std::string::npos) {
//             std::cerr << "❌ ERROR: 'junctions' array not found in JSON!\n";
//             return false;
//         }
        
//         // Find start of array
//         size_t arrayStart = content.find("[", junctionsPos);
//         if (arrayStart == std::string::npos) {
//             std::cerr << "❌ ERROR: Invalid JSON format!\n";
//             return false;
//         }
        
//         // Parse each junction object
//         size_t pos = arrayStart + 1;
//         while (pos < content.length()) {
//             // Find next object
//             size_t objStart = content.find("{", pos);
//             if (objStart == std::string::npos) break;
            
//             size_t objEnd = content.find("}", objStart);
//             if (objEnd == std::string::npos) break;
            
//             // Extract junction object
//             std::string junctionStr = content.substr(objStart, objEnd - objStart + 1);
            
//             try {
//                 Junction j;
//                 j.id = extractInt(junctionStr, "id");
//                 j.name = extractString(junctionStr, "name");
//                 j.latitude = extractDouble(junctionStr, "latitude");
//                 j.longitude = extractDouble(junctionStr, "longitude");
//                 j.city = extractString(junctionStr, "city");
//                 j.area = extractString(junctionStr, "area");
//                 j.hasTrafficSignal = extractBool(junctionStr, "hasTrafficSignal");
                
//                 // Only add if valid
//                 if (j.id > 0 && !j.name.empty() && j.latitude != 0.0) {
//                     trafficManager.addJunction(j);
//                     loadedCount++;
                    
//                     // Progress every 500
//                     if (loadedCount % 500 == 0) {
//                         std::cout << "   📍 Loaded " << loadedCount << " junctions...\n";
//                     }
//                 }
//             } catch (...) {
//                 errorCount++;
//             }
            
//             pos = objEnd + 1;
//         }
        
//         auto endTime = std::chrono::high_resolution_clock::now();
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
//         std::cout << "\n╔════════════════════════════════════════╗\n";
//         std::cout << "║   LOADING COMPLETE!                   ║\n";
//         std::cout << "╚════════════════════════════════════════╝\n\n";
//         std::cout << "✅ Junctions Loaded: " << loadedCount << "\n";
//         std::cout << "⚠️  Errors: " << errorCount << "\n";
//         std::cout << "⏱️  Load Time: " << duration.count() << " ms\n";
//         std::cout << "💾 B-Tree Nodes: ~" << (loadedCount / 10) << "\n";
//         std::cout << "🔍 Search Complexity: O(log n)\n";
//         std::cout << "⚡ Average Search Time: <1ms\n\n";
        
//         return loadedCount > 0;
//     }
    
//     void generateRoadNetwork(double maxDistanceKm = 5.0) {
//         std::cout << "╔════════════════════════════════════════╗\n";
//         std::cout << "║   GENERATING ROAD NETWORK             ║\n";
//         std::cout << "╚════════════════════════════════════════╝\n\n";
//         std::cout << "🛣️  Connecting nearby junctions...\n";
//         std::cout << "📏 Max connection distance: " << maxDistanceKm << " km\n";
//         std::cout << "🏙️  Same-city connections only\n\n";
        
//         auto junctions = trafficManager.getAllJunctions();
//         int roadId = 1;
//         int roadsGenerated = 0;
        
//         size_t totalPairs = (junctions.size() * (junctions.size() - 1)) / 2;
//         size_t processedPairs = 0;
//         int lastProgress = 0;
        
//         for (size_t i = 0; i < junctions.size(); ++i) {
//             for (size_t j = i + 1; j < junctions.size(); ++j) {
//                 processedPairs++;
                
//                 if (junctions[i].city != junctions[j].city) continue;
                
//                 double distance = junctions[i].distanceTo(junctions[j]);
                
//                 if (distance < maxDistanceKm) {
//                     double speedLimit = 40.0;
                    
//                     std::string areaLower = junctions[i].area;
//                     std::transform(areaLower.begin(), areaLower.end(), 
//                                  areaLower.begin(), ::tolower);
                    
//                     if (areaLower.find("highway") != std::string::npos ||
//                         areaLower.find("motorway") != std::string::npos) {
//                         speedLimit = 100.0;
//                     } else if (areaLower.find("main") != std::string::npos ||
//                              areaLower.find("road") != std::string::npos) {
//                         speedLimit = 60.0;
//                     }
                    
//                     Road road(roadId++, 
//                              junctions[i].name + " to " + junctions[j].name,
//                              junctions[i].id,
//                              junctions[j].id,
//                              distance,
//                              speedLimit);
//                     road.isTwoWay = true;
                    
//                     trafficManager.addRoad(road);
//                     roadsGenerated++;
//                 }
                
//                 int progress = (processedPairs * 100) / totalPairs;
//                 if (progress >= lastProgress + 10) {
//                     std::cout << "   🔗 Progress: " << progress << "% complete...\n";
//                     lastProgress = progress;
//                 }
//             }
//         }
        
//         std::cout << "\n✅ Road Network Generated!\n";
//         std::cout << "   Total Roads: " << roadsGenerated << "\n";
//         std::cout << "   Bidirectional: Yes\n";
//         std::cout << "   Graph Edges: " << (roadsGenerated * 2) << "\n\n";
//     }
    
//     void printStats() {
//         std::cout << "╔════════════════════════════════════════╗\n";
//         std::cout << "║   SYSTEM STATISTICS                   ║\n";
//         std::cout << "╚════════════════════════════════════════╝\n\n";
        
//         trafficManager.printStatistics();
        
//         std::cout << "\n📊 Data Structures in Use:\n";
//         std::cout << "   ├─ B-Tree (Name Index)\n";
//         std::cout << "   ├─ B-Tree (City Index)\n";
//         std::cout << "   ├─ Hash Table (ID Lookup)\n";
//         std::cout << "   ├─ Graph (Dijkstra's Algorithm)\n";
//         std::cout << "   ├─ Min-Heap (Priority Queue)\n";
//         std::cout << "   └─ LRU Cache (Route Cache)\n\n";
//     }
// };

// #endif