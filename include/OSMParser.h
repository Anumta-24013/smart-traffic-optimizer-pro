/**
 * Smart Traffic Route Optimizer
 * OSM Parser - Load Real OpenStreetMap Data
 * 
 * Parses OSM XML files and loads junctions + roads
 */

#ifndef OSMPARSER_H
#define OSMPARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include "Models.h"

class OSMParser {
private:
    struct OSMNode {
        long long id;
        double lat;
        double lon;
        std::string name;
    };
    
    struct OSMWay {
        long long id;
        std::vector<long long> nodes;
        std::string name;
        std::string type;  // highway, primary, secondary, etc.
    };
    
    std::map<long long, OSMNode> nodes;
    std::vector<OSMWay> ways;
    
    // Extract value from XML tag
    std::string extractAttribute(const std::string& line, const std::string& attr) {
        size_t pos = line.find(attr + "=\"");
        if (pos == std::string::npos) return "";
        
        pos += attr.length() + 2;
        size_t end = line.find("\"", pos);
        if (end == std::string::npos) return "";
        
        return line.substr(pos, end - pos);
    }
    
    // Check if way is a road
    bool isRoad(const std::string& type) {
        return (type == "motorway" || type == "trunk" || 
                type == "primary" || type == "secondary" ||
                type == "tertiary" || type == "residential" ||
                type == "unclassified");
    }

public:
    // Parse OSM XML file
    bool parseOSMFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "❌ Cannot open OSM file: " << filename << "\n";
            return false;
        }
        
        std::cout << "📂 Reading OSM file: " << filename << "\n";
        
        std::string line;
        OSMNode currentNode;
        OSMWay currentWay;
        bool inNode = false;
        bool inWay = false;
        
        int nodeCount = 0;
        int wayCount = 0;
        
        while (std::getline(file, line)) {
            // Parse Node (junction/intersection)
            if (line.find("<node ") != std::string::npos) {
                inNode = true;
                
                std::string idStr = extractAttribute(line, "id");
                std::string latStr = extractAttribute(line, "lat");
                std::string lonStr = extractAttribute(line, "lon");
                
                if (!idStr.empty() && !latStr.empty() && !lonStr.empty()) {
                    currentNode.id = std::stoll(idStr);
                    currentNode.lat = std::stod(latStr);
                    currentNode.lon = std::stod(lonStr);
                    currentNode.name = "";
                    nodeCount++;
                }
            }
            
            // Parse node name tag
            else if (inNode && line.find("<tag k=\"name\"") != std::string::npos) {
                currentNode.name = extractAttribute(line, "v");
            }
            
            // End of node
            else if (inNode && (line.find("</node>") != std::string::npos || 
                               line.find("/>") != std::string::npos)) {
                nodes[currentNode.id] = currentNode;
                inNode = false;
            }
            
            // Parse Way (road)
            else if (line.find("<way ") != std::string::npos) {
                inWay = true;
                currentWay.nodes.clear();
                currentWay.name = "";
                currentWay.type = "";
                
                std::string idStr = extractAttribute(line, "id");
                if (!idStr.empty()) {
                    currentWay.id = std::stoll(idStr);
                }
            }
            
            // Parse way node reference
            else if (inWay && line.find("<nd ref=") != std::string::npos) {
                std::string refStr = extractAttribute(line, "ref");
                if (!refStr.empty()) {
                    currentWay.nodes.push_back(std::stoll(refStr));
                }
            }
            
            // Parse way name
            else if (inWay && line.find("<tag k=\"name\"") != std::string::npos) {
                currentWay.name = extractAttribute(line, "v");
            }
            
            // Parse way highway type
            else if (inWay && line.find("<tag k=\"highway\"") != std::string::npos) {
                currentWay.type = extractAttribute(line, "v");
            }
            
            // End of way
            else if (inWay && line.find("</way>") != std::string::npos) {
                if (isRoad(currentWay.type) && currentWay.nodes.size() >= 2) {
                    ways.push_back(currentWay);
                    wayCount++;
                }
                inWay = false;
            }
        }
        
        file.close();
        
        std::cout << "✅ Parsed " << nodeCount << " nodes\n";
        std::cout << "✅ Parsed " << wayCount << " roads\n";
        
        return true;
    }
    
    // Convert OSM data to Junction objects
    std::vector<Junction> getJunctions(int maxJunctions = 1000) {
        std::vector<Junction> junctions;
        int junctionId = 1;
        
        std::cout << "🔄 Converting OSM nodes to junctions...\n";
        
        // Create junctions from nodes that are part of roads
        std::map<long long, bool> usedNodes;
        
        // Mark nodes used in ways
        for (const auto& way : ways) {
            for (long long nodeId : way.nodes) {
                usedNodes[nodeId] = true;
            }
        }
        
        // Convert used nodes to junctions
        for (const auto& pair : nodes) {
            if (usedNodes[pair.first]) {
                Junction j;
                j.id = junctionId++;
                j.name = pair.second.name.empty() ? 
                         "Junction " + std::to_string(j.id) : 
                         pair.second.name;
                j.latitude = pair.second.lat;
                j.longitude = pair.second.lon;
                j.city = "Lahore";  // You can detect city from coordinates
                j.area = detectArea(pair.second.lat, pair.second.lon);
                
                junctions.push_back(j);
                
                if (junctions.size() >= maxJunctions) break;
            }
        }
        
        std::cout << "✅ Created " << junctions.size() << " junctions\n";
        return junctions;
    }
    
    // Detect area from coordinates (simple version)
    std::string detectArea(double lat, double lon) {
        // Lahore coordinates roughly: 31.4-31.6 lat, 74.2-74.4 lon
        
        if (lat > 31.52 && lon > 74.35) return "Model Town";
        else if (lat > 31.52 && lon < 74.35) return "Garden Town";
        else if (lat < 31.52 && lon > 74.35) return "DHA";
        else if (lat < 31.52 && lon < 74.35) return "Gulberg";
        else return "Central Lahore";
    }
    
    // Convert OSM ways to Road objects
    std::vector<Road> getRoads(const std::map<int, long long>& junctionOSMMap) {
        std::vector<Road> roads;
        int roadId = 1;
        
        std::cout << "🔄 Converting OSM ways to roads...\n";
        
        for (const auto& way : ways) {
            // Connect consecutive nodes in the way
            for (size_t i = 0; i < way.nodes.size() - 1; i++) {
                long long osmNode1 = way.nodes[i];
                long long osmNode2 = way.nodes[i + 1];
                
                // Find corresponding junction IDs
                int junc1 = -1, junc2 = -1;
                for (const auto& pair : junctionOSMMap) {
                    if (pair.second == osmNode1) junc1 = pair.first;
                    if (pair.second == osmNode2) junc2 = pair.first;
                }
                
                if (junc1 != -1 && junc2 != -1) {
                    Road road;
                    road.id = roadId++;
                    road.sourceJunction = junc1;
                    road.destJunction = junc2;
                    road.name = way.name.empty() ? "Road " + std::to_string(road.id) : way.name;
                    
                    // Calculate distance using Haversine formula
                    road.distance = calculateDistance(
                        nodes[osmNode1].lat, nodes[osmNode1].lon,
                        nodes[osmNode2].lat, nodes[osmNode2].lon
                    );
                    
                    road.baseTime = road.distance / 40.0;  // Assume 40 km/h avg speed
                    road.trafficLevel = TrafficLevel::NORMAL;
                    road.isTwoWay = true;
                    
                    roads.push_back(road);
                }
            }
        }
        
        std::cout << "✅ Created " << roads.size() << " roads\n";
        return roads;
    }
    
    // Haversine formula for distance calculation
    double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
        const double R = 6371.0;  // Earth radius in km
        
        double dLat = (lat2 - lat1) * M_PI / 180.0;
        double dLon = (lon2 - lon1) * M_PI / 180.0;
        
        double a = sin(dLat/2) * sin(dLat/2) +
                   cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
                   sin(dLon/2) * sin(dLon/2);
        
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        return R * c;
    }
    
    // Get statistics
    void printStats() {
        std::cout << "\n📊 OSM Data Statistics:\n";
        std::cout << "   Total Nodes: " << nodes.size() << "\n";
        std::cout << "   Total Ways: " << ways.size() << "\n";
    }
};

#endif // OSMPARSER_H

