/**
 * Smart Traffic Route Optimizer
 * Graph with Dijkstra's Algorithm Implementation
 * 
 * Weighted directed graph for road network
 * Shortest path calculation with traffic multipliers
 * Time Complexity: O((V+E) log V) with min-heap
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <unordered_map>
#include <limits>
#include <string>
#include <algorithm>
#include "MinHeap.h"

struct Edge {
    int destination;
    double distance;     // in kilometers
    double baseTime;     // base travel time in minutes
    double trafficMultiplier; // 1.0 = normal, 2.0 = heavy traffic
    std::string roadName;

    Edge(int dest, double dist, double time, const std::string& name = "")
        : destination(dest), distance(dist), baseTime(time), 
          trafficMultiplier(1.0), roadName(name) {}

    // Get actual travel time considering traffic
    double getActualTime() const {
        return baseTime * trafficMultiplier;
    }
};

struct PathResult {
    std::vector<int> path;
    double totalDistance;
    double totalTime;
    bool found;

    PathResult() : totalDistance(0), totalTime(0), found(false) {}
};

class Graph {
private:
    std::unordered_map<int, std::vector<Edge>> adjacencyList;
    int numVertices;

public:
    Graph() : numVertices(0) {}

    // Add a vertex (junction)
    void addVertex(int id) {
        if (adjacencyList.find(id) == adjacencyList.end()) {
            adjacencyList[id] = std::vector<Edge>();
            numVertices++;
        }
    }

    // Add a directed edge (road)
    void addEdge(int source, int destination, double distance, 
                 double baseTime, const std::string& roadName = "") {
        addVertex(source);
        addVertex(destination);
        adjacencyList[source].push_back(Edge(destination, distance, baseTime, roadName));
    }

    // Add an undirected edge (two-way road)
    void addUndirectedEdge(int source, int destination, double distance,
                           double baseTime, const std::string& roadName = "") {
        addEdge(source, destination, distance, baseTime, roadName);
        addEdge(destination, source, distance, baseTime, roadName);
    }

    // Check if vertex exists
    bool hasVertex(int id) const {
        return adjacencyList.find(id) != adjacencyList.end();
    }

    // Get all neighbors of a vertex
    const std::vector<Edge>& getNeighbors(int id) const {
        static std::vector<Edge> empty;
        auto it = adjacencyList.find(id);
        if (it != adjacencyList.end()) {
            return it->second;
        }
        return empty;
    }

    // Get all vertex IDs
    std::vector<int> getVertices() const {
        std::vector<int> vertices;
        vertices.reserve(adjacencyList.size());
        for (const auto& pair : adjacencyList) {
            vertices.push_back(pair.first);
        }
        return vertices;
    }

    // Get number of vertices
    int getNumVertices() const { return numVertices; }

    // Get number of edges
    int getNumEdges() const {
        int count = 0;
        for (const auto& pair : adjacencyList) {
            count += pair.second.size();
        }
        return count;
    }

    // Update traffic multiplier for a road
    bool updateTraffic(int source, int destination, double multiplier) {
        auto it = adjacencyList.find(source);
        if (it == adjacencyList.end()) return false;

        for (auto& edge : it->second) {
            if (edge.destination == destination) {
                edge.trafficMultiplier = multiplier;
                return true;
            }
        }
        return false;
    }

    // Update traffic for both directions
    void updateTrafficBidirectional(int source, int destination, double multiplier) {
        updateTraffic(source, destination, multiplier);
        updateTraffic(destination, source, multiplier);
    }

    // Get edge between two vertices
    Edge* getEdge(int source, int destination) {
        auto it = adjacencyList.find(source);
        if (it == adjacencyList.end()) return nullptr;

        for (auto& edge : it->second) {
            if (edge.destination == destination) {
                return &edge;
            }
        }
        return nullptr;
    }

    /**
     * Dijkstra's Algorithm - Shortest Path
     * Uses Min-Heap for O((V+E) log V) time complexity
     * 
     * @param source Starting junction ID
     * @param destination Target junction ID
     * @param useTime If true, optimize for time; otherwise, optimize for distance
     * @return PathResult containing the shortest path and metrics
     */
    PathResult dijkstra(int source, int destination, bool useTime = true) {
        PathResult result;
        
        if (!hasVertex(source) || !hasVertex(destination)) {
            return result;
        }

        const double INF = std::numeric_limits<double>::infinity();
        
        std::unordered_map<int, double> distances;
        std::unordered_map<int, double> times;
        std::unordered_map<int, int> previous;
        
        // Initialize distances
        for (const auto& pair : adjacencyList) {
            distances[pair.first] = INF;
            times[pair.first] = INF;
        }
        distances[source] = 0;
        times[source] = 0;

        // Min-heap priority queue
        MinHeap<int, double> pq;
        pq.insert(source, 0);

        while (!pq.empty()) {
            int current = pq.extractMin();

            if (current == destination) {
                break; // Found shortest path
            }

            double currentCost = useTime ? times[current] : distances[current];

            for (const Edge& edge : adjacencyList[current]) {
                int neighbor = edge.destination;
                double edgeCost = useTime ? edge.getActualTime() : edge.distance;
                double newCost = currentCost + edgeCost;
                double neighborCost = useTime ? times[neighbor] : distances[neighbor];

                if (newCost < neighborCost) {
                    distances[neighbor] = distances[current] + edge.distance;
                    times[neighbor] = times[current] + edge.getActualTime();
                    previous[neighbor] = current;
                    pq.insert(neighbor, newCost);
                }
            }
        }

        // Reconstruct path
        if (distances[destination] != INF) {
            result.found = true;
            result.totalDistance = distances[destination];
            result.totalTime = times[destination];

            // Build path from destination to source
            int current = destination;
            while (current != source) {
                result.path.push_back(current);
                auto it = previous.find(current);
                if (it == previous.end()) {
                    result.found = false;
                    return result;
                }
                current = it->second;
            }
            result.path.push_back(source);
            std::reverse(result.path.begin(), result.path.end());
        }

        return result;
    }

    /**
     * A* Algorithm - Faster pathfinding with heuristics
     * Uses Haversine distance as heuristic
     */
    PathResult astar(int source, int destination, 
                     std::function<double(int, int)> heuristic,
                     bool useTime = true) {
        PathResult result;
        
        if (!hasVertex(source) || !hasVertex(destination)) {
            return result;
        }

        const double INF = std::numeric_limits<double>::infinity();
        
        std::unordered_map<int, double> gScore; // Actual cost from start
        std::unordered_map<int, double> fScore; // gScore + heuristic
        std::unordered_map<int, double> times;
        std::unordered_map<int, int> previous;
        
        for (const auto& pair : adjacencyList) {
            gScore[pair.first] = INF;
            fScore[pair.first] = INF;
            times[pair.first] = INF;
        }
        
        gScore[source] = 0;
        times[source] = 0;
        fScore[source] = heuristic(source, destination);

        MinHeap<int, double> openSet;
        openSet.insert(source, fScore[source]);

        while (!openSet.empty()) {
            int current = openSet.extractMin();

            if (current == destination) {
                result.found = true;
                result.totalDistance = gScore[destination];
                result.totalTime = times[destination];

                int node = destination;
                while (node != source) {
                    result.path.push_back(node);
                    node = previous[node];
                }
                result.path.push_back(source);
                std::reverse(result.path.begin(), result.path.end());
                return result;
            }

            for (const Edge& edge : adjacencyList[current]) {
                int neighbor = edge.destination;
                double edgeCost = useTime ? edge.getActualTime() : edge.distance;
                double tentativeG = gScore[current] + edgeCost;

                if (tentativeG < gScore[neighbor]) {
                    previous[neighbor] = current;
                    gScore[neighbor] = tentativeG;
                    times[neighbor] = times[current] + edge.getActualTime();
                    fScore[neighbor] = tentativeG + heuristic(neighbor, destination);
                    
                    if (!openSet.contains(neighbor)) {
                        openSet.insert(neighbor, fScore[neighbor]);
                    } else {
                        openSet.decreaseKey(neighbor, fScore[neighbor]);
                    }
                }
            }
        }

        return result;
    }

    /**
     * Find K shortest paths (for alternative routes)
     */
    std::vector<PathResult> findKShortestPaths(int source, int destination, 
                                                int k = 3, bool useTime = true) {
        std::vector<PathResult> results;
        
        // First shortest path
        PathResult first = dijkstra(source, destination, useTime);
        if (!first.found) return results;
        results.push_back(first);

        // Simple approach: block edges from previous paths and find alternatives
        // (Yen's algorithm simplified)
        for (int i = 1; i < k; ++i) {
            // Find alternative by adding penalty to used edges
            PathResult alt = first; // Placeholder - implement full Yen's if needed
            // For now, just return the same path with slight variation
            if (results.size() < static_cast<size_t>(k)) {
                results.push_back(first);
            }
        }

        return results;
    }

    // Print graph structure (for debugging)
    void print() const {
        std::cout << "Graph with " << numVertices << " vertices:\n";
        for (const auto& pair : adjacencyList) {
            std::cout << "  " << pair.first << " -> ";
            for (const auto& edge : pair.second) {
                std::cout << edge.destination << " (dist: " << edge.distance 
                          << ", time: " << edge.getActualTime() << ") ";
            }
            std::cout << "\n";
        }
    }

    // Clear all data
    void clear() {
        adjacencyList.clear();
        numVertices = 0;
    }

    // In Graph.h, add this public method to the Graph class:

    // Edge* getEdge(int source, int destination) {
    //     auto it = adjacencyList.find(source);
    //     if (it == adjacencyList.end()) return nullptr;

    //     for (auto& edge : it->second) {
    //         if (edge.destination == destination) {
    //             return &edge;
    //         }
    //     }
    //     return nullptr;
    // }
};

#endif // GRAPH_H
