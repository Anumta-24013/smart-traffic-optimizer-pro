/**
 * Smart Traffic Route Optimizer
 * Main Entry Point
 * 
 * Usage:
 *   ./traffic_optimizer           - Run CLI application
 *   ./traffic_optimizer --server  - Run HTTP API server
 *   ./traffic_optimizer --test    - Run tests
 */

#include <iostream>
#include <string>
#include <limits>
#include <cstring>
#include "include/TrafficManager.h"
#include "include/HttpServer.h"

// Global traffic manager
TrafficManager trafficManager(100);

// Initialize sample data
void initializeLahoreData() {
    std::cout << "Loading Lahore traffic data...\n";

    // Add Junctions
    trafficManager.addJunction(Junction(1, "Liberty Chowk", 31.5104, 74.3416, "Lahore", "Gulberg"));
    trafficManager.addJunction(Junction(2, "Mall Road Chowk", 31.5500, 74.3440, "Lahore", "Mall Road"));
    trafficManager.addJunction(Junction(3, "Kalma Chowk", 31.5158, 74.3294, "Lahore", "Gulberg"));
    trafficManager.addJunction(Junction(4, "Faisal Chowk", 31.5580, 74.3172, "Lahore", "Faisal Town"));
    trafficManager.addJunction(Junction(5, "Thokar Niaz Baig", 31.4711, 74.2675, "Lahore", "Thokar"));
    trafficManager.addJunction(Junction(6, "Defence Mor", 31.4795, 74.3848, "Lahore", "DHA"));
    trafficManager.addJunction(Junction(7, "Model Town Link Road", 31.4820, 74.3156, "Lahore", "Model Town"));
    trafficManager.addJunction(Junction(8, "Barkat Market", 31.5021, 74.3256, "Lahore", "Garden Town"));
    trafficManager.addJunction(Junction(9, "Chungi Amar Sidhu", 31.4548, 74.2903, "Lahore", "Chungi"));
    trafficManager.addJunction(Junction(10, "Data Darbar", 31.5701, 74.3141, "Lahore", "Old Lahore"));
    trafficManager.addJunction(Junction(11, "Lahore Railway Station", 31.5748, 74.3086, "Lahore", "Old Lahore"));
    trafficManager.addJunction(Junction(12, "Gaddafi Stadium", 31.5127, 74.3358, "Lahore", "Gulberg"));

    // Add Roads
    Road r1(1, "Main Boulevard Gulberg", 1, 3, 2.5, 50); r1.isTwoWay = true;
    Road r2(2, "Ferozepur Road", 3, 5, 6.0, 60); r2.isTwoWay = true;
    Road r3(3, "Liberty to Defence", 1, 6, 4.5, 40); r3.isTwoWay = true;
    Road r4(4, "Canal Road", 3, 4, 5.0, 70); r4.isTwoWay = true;
    Road r5(5, "Model Town Road", 5, 7, 3.5, 50); r5.isTwoWay = true;
    Road r6(6, "Garden Town Road", 7, 8, 2.0, 40); r6.isTwoWay = true;
    Road r7(7, "Main Market Road", 8, 1, 1.5, 30); r7.isTwoWay = true;
    Road r8(8, "Ferozepur Road South", 5, 9, 2.5, 60); r8.isTwoWay = true;
    Road r9(9, "Mall Road", 2, 10, 3.0, 40); r9.isTwoWay = true;
    Road r10(10, "Railway Road", 10, 11, 1.0, 30); r10.isTwoWay = true;
    Road r11(11, "Gulberg Link", 1, 12, 1.2, 40); r11.isTwoWay = true;
    Road r12(12, "Stadium Road", 12, 3, 1.0, 40); r12.isTwoWay = true;
    Road r13(13, "Mall to Gulberg", 2, 4, 4.0, 50); r13.isTwoWay = true;
    Road r14(14, "Defence Link Road", 6, 7, 3.0, 50); r14.isTwoWay = true;
    Road r15(15, "Chungi to Model Town", 9, 7, 4.0, 50); r15.isTwoWay = true;

    trafficManager.addRoad(r1);
    trafficManager.addRoad(r2);
    trafficManager.addRoad(r3);
    trafficManager.addRoad(r4);
    trafficManager.addRoad(r5);
    trafficManager.addRoad(r6);
    trafficManager.addRoad(r7);
    trafficManager.addRoad(r8);
    trafficManager.addRoad(r9);
    trafficManager.addRoad(r10);
    trafficManager.addRoad(r11);
    trafficManager.addRoad(r12);
    trafficManager.addRoad(r13);
    trafficManager.addRoad(r14);
    trafficManager.addRoad(r15);

    std::cout << "Loaded " << trafficManager.getJunctionCount() << " junctions and "
              << trafficManager.getRoadCount() << " roads.\n";
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printBanner() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║       🚗  SMART TRAFFIC ROUTE OPTIMIZER  🚗                  ║\n";
    std::cout << "║                    Pakistan Cities                            ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void printMenu() {
    std::cout << "┌────────────────────────────────────────┐\n";
    std::cout << "│             MAIN MENU                  │\n";
    std::cout << "├────────────────────────────────────────┤\n";
    std::cout << "│  1. View All Junctions                 │\n";
    std::cout << "│  2. Find Shortest Route                │\n";
    std::cout << "│  3. Update Traffic Level               │\n";
    std::cout << "│  4. Search Junction by Name            │\n";
    std::cout << "│  5. View Road Network                  │\n";
    std::cout << "│  6. View System Statistics             │\n";
    std::cout << "│  7. Start API Server                   │\n";
    std::cout << "│  0. Exit                               │\n";
    std::cout << "└────────────────────────────────────────┘\n";
    std::cout << "\nEnter your choice: ";
}

void viewAllJunctions() {
    clearScreen();
    printBanner();
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    ALL JUNCTIONS                               \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";

    auto junctions = trafficManager.getAllJunctions();
    
    std::cout << "┌──────┬────────────────────────────┬─────────────────┬─────────────┐\n";
    std::cout << "│  ID  │           Name             │      Area       │    City     │\n";
    std::cout << "├──────┼────────────────────────────┼─────────────────┼─────────────┤\n";
    
    for (const auto& j : junctions) {
        printf("│ %4d │ %-26s │ %-15s │ %-11s │\n", 
               j.id, j.name.substr(0, 26).c_str(), 
               j.area.substr(0, 15).c_str(), j.city.substr(0, 11).c_str());
    }
    
    std::cout << "└──────┴────────────────────────────┴─────────────────┴─────────────┘\n";
    std::cout << "\nTotal Junctions: " << junctions.size() << "\n";
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void findShortestRoute() {
    clearScreen();
    printBanner();
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    FIND SHORTEST ROUTE                         \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";

    // Show available junctions
    auto junctions = trafficManager.getAllJunctions();
    std::cout << "Available Junctions:\n";
    for (const auto& j : junctions) {
        std::cout << "  " << j.id << ". " << j.name << " (" << j.area << ")\n";
    }
    std::cout << "\n";

    int fromId, toId;
    std::cout << "Enter Source Junction ID: ";
    std::cin >> fromId;
    std::cout << "Enter Destination Junction ID: ";
    std::cin >> toId;

    int optimize;
    std::cout << "\nOptimize for:\n";
    std::cout << "  1. Fastest Route (Time)\n";
    std::cout << "  2. Shortest Route (Distance)\n";
    std::cout << "Choice: ";
    std::cin >> optimize;

    bool useTime = (optimize == 1);
    
    std::cout << "\n🔍 Calculating route...\n\n";
    
    RouteResult result = trafficManager.findRoute(fromId, toId, useTime);

    if (result.found) {
        std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                    ✅ ROUTE FOUND!                            ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

        std::cout << "📏 Total Distance: " << result.totalDistance << " km\n";
        
        int hours = static_cast<int>(result.totalTime / 60);
        int minutes = static_cast<int>(result.totalTime) % 60;
        std::cout << "⏱️  Estimated Time: ";
        if (hours > 0) std::cout << hours << "h ";
        std::cout << minutes << " minutes\n\n";

        std::cout << "📍 Route Path:\n";
        std::cout << "┌─────────────────────────────────────────────────────────────┐\n";
        
        for (size_t i = 0; i < result.junctions.size(); ++i) {
            std::cout << "│  " << (i + 1) << ". " << result.junctions[i].name;
            std::cout << " (" << result.junctions[i].area << ")";
            std::cout << std::string(40 - result.junctions[i].name.length() - result.junctions[i].area.length(), ' ');
            std::cout << "│\n";
            
            if (i < result.junctions.size() - 1) {
                std::cout << "│       ↓                                                    │\n";
            }
        }
        
        std::cout << "└─────────────────────────────────────────────────────────────┘\n";
    } else {
        std::cout << "❌ No route found between the selected junctions.\n";
    }

    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void updateTrafficLevel() {
    clearScreen();
    printBanner();
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    UPDATE TRAFFIC LEVEL                        \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";

    // Show available roads
    auto roads = trafficManager.getAllRoads();
    std::cout << "Available Roads:\n";
    std::cout << "┌──────┬────────────────────────────┬───────────────────────────┐\n";
    std::cout << "│  ID  │         Road Name          │      Current Traffic      │\n";
    std::cout << "├──────┼────────────────────────────┼───────────────────────────┤\n";
    
    for (const auto& r : roads) {
        printf("│ %4d │ %-26s │ %-25s │\n", 
               r.id, r.name.substr(0, 26).c_str(), 
               trafficLevelToString(r.trafficLevel).c_str());
    }
    std::cout << "└──────┴────────────────────────────┴───────────────────────────┘\n\n";

    int roadId;
    std::cout << "Enter Road ID to update: ";
    std::cin >> roadId;

    std::cout << "\nSelect Traffic Level:\n";
    std::cout << "  1. 🟢 Low (Free flowing)\n";
    std::cout << "  2. 🟡 Normal\n";
    std::cout << "  3. 🟠 Heavy\n";
    std::cout << "  4. 🔴 Severe (Congestion)\n";
    std::cout << "Choice: ";

    int level;
    std::cin >> level;

    TrafficLevel trafficLevel;
    switch (level) {
        case 1: trafficLevel = TrafficLevel::LOW; break;
        case 2: trafficLevel = TrafficLevel::NORMAL; break;
        case 3: trafficLevel = TrafficLevel::HEAVY; break;
        case 4: trafficLevel = TrafficLevel::SEVERE; break;
        default: trafficLevel = TrafficLevel::NORMAL;
    }

    if (trafficManager.updateTrafficLevel(roadId, trafficLevel)) {
        std::cout << "\n✅ Traffic level updated successfully!\n";
    } else {
        std::cout << "\n❌ Road not found.\n";
    }

    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void searchJunction() {
    clearScreen();
    printBanner();
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    SEARCH JUNCTION                             \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";

    std::string query;
    std::cout << "Enter search query: ";
    std::cin.ignore();
    std::getline(std::cin, query);

    auto results = trafficManager.searchJunctions(query);

    if (results.empty()) {
        std::cout << "\n❌ No junctions found matching \"" << query << "\"\n";
    } else {
        std::cout << "\n✅ Found " << results.size() << " junction(s):\n\n";
        
        for (const auto& j : results) {
            std::cout << "┌─────────────────────────────────────────────────────────────┐\n";
            std::cout << "│ ID: " << j.id << "\n";
            std::cout << "│ Name: " << j.name << "\n";
            std::cout << "│ Area: " << j.area << ", " << j.city << "\n";
            std::cout << "│ Coordinates: " << j.latitude << ", " << j.longitude << "\n";
            std::cout << "└─────────────────────────────────────────────────────────────┘\n";
        }
    }

    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void viewRoadNetwork() {
    clearScreen();
    printBanner();
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    ROAD NETWORK                                \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";

    auto roads = trafficManager.getAllRoads();
    
    for (const auto& r : roads) {
        Junction source, dest;
        trafficManager.getJunction(r.sourceJunction, &source);
        trafficManager.getJunction(r.destJunction, &dest);
        
        std::cout << "┌─────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ 🛣️  " << r.name << "\n";
        std::cout << "│ " << source.name << " ←→ " << dest.name << "\n";
        std::cout << "│ Distance: " << r.distance << " km | Speed Limit: " << r.speedLimit << " km/h\n";
        std::cout << "│ Traffic: " << trafficLevelToString(r.trafficLevel);
        std::cout << " (×" << getTrafficMultiplier(r.trafficLevel) << ")\n";
        std::cout << "│ Est. Time: " << r.getActualTime() << " minutes\n";
        std::cout << "└─────────────────────────────────────────────────────────────┘\n";
    }

    std::cout << "\nTotal Roads: " << roads.size() << "\n";
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void viewStatistics() {
    clearScreen();
    printBanner();
    trafficManager.printStatistics();
    
    std::cout << "\nData Structures Used:\n";
    std::cout << "  ├─ B-Tree: Junction name indexing (O(log n) search)\n";
    std::cout << "  ├─ Hash Table: Junction ID lookup (O(1) average)\n";
    std::cout << "  ├─ Graph: Road network with weighted edges\n";
    std::cout << "  ├─ Min-Heap: Dijkstra's algorithm optimization\n";
    std::cout << "  └─ LRU Cache: Route result caching\n";
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void startServer() {
    clearScreen();
    printBanner();
    
    int port = 8080;
    std::cout << "Starting HTTP API Server on port " << port << "...\n\n";
    
    HttpServer server(port, trafficManager);
    if (server.start()) {
        server.run();
    } else {
        std::cout << "❌ Failed to start server.\n";
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
}

void runCLI() {
    int choice;
    
    do {
        clearScreen();
        printBanner();
        printMenu();
        std::cin >> choice;

        switch (choice) {
            case 1: viewAllJunctions(); break;
            case 2: findShortestRoute(); break;
            case 3: updateTrafficLevel(); break;
            case 4: searchJunction(); break;
            case 5: viewRoadNetwork(); break;
            case 6: viewStatistics(); break;
            case 7: startServer(); break;
            case 0:
                clearScreen();
                printBanner();
                std::cout << "Thank you for using Smart Traffic Route Optimizer!\n";
                std::cout << "Goodbye! 👋\n\n";
                break;
            default:
                std::cout << "\n⚠️  Invalid choice. Please try again.\n";
                std::cout << "Press Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
        }
    } while (choice != 0);
}

void runTests() {
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    RUNNING TESTS                               \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";

    // Test 1: B-Tree
    std::cout << "Test 1: B-Tree Operations... ";
    BTree<std::string, int> btree;
    btree.insert("test1", 1);
    btree.insert("test2", 2);
    btree.insert("test3", 3);
    int val;
    bool passed = btree.search("test2", &val) && val == 2;
    std::cout << (passed ? "✅ PASSED" : "❌ FAILED") << "\n";

    // Test 2: Hash Table
    std::cout << "Test 2: Hash Table Operations... ";
    HashTable<int, std::string> ht;
    ht.insert(1, "one");
    ht.insert(2, "two");
    std::string htVal;
    passed = ht.search(2, &htVal) && htVal == "two";
    std::cout << (passed ? "✅ PASSED" : "❌ FAILED") << "\n";

    // Test 3: Min-Heap
    std::cout << "Test 3: Min-Heap Operations... ";
    MinHeap<int, double> heap;
    heap.insert(3, 3.0);
    heap.insert(1, 1.0);
    heap.insert(2, 2.0);
    passed = heap.extractMin() == 1 && heap.extractMin() == 2;
    std::cout << (passed ? "✅ PASSED" : "❌ FAILED") << "\n";

    // Test 4: LRU Cache
    std::cout << "Test 4: LRU Cache Operations... ";
    LRUCache<std::string, int> cache(2);
    cache.put("a", 1);
    cache.put("b", 2);
    cache.put("c", 3); // Should evict "a"
    int cacheVal;
    passed = !cache.get("a", &cacheVal) && cache.get("b", &cacheVal);
    std::cout << (passed ? "✅ PASSED" : "❌ FAILED") << "\n";

    // Test 5: Graph + Dijkstra
    std::cout << "Test 5: Dijkstra's Algorithm... ";
    Graph g;
    g.addUndirectedEdge(1, 2, 1.0, 1.0);
    g.addUndirectedEdge(2, 3, 1.0, 1.0);
    g.addUndirectedEdge(1, 3, 3.0, 3.0);
    PathResult path = g.dijkstra(1, 3, false);
    passed = path.found && path.path.size() == 3 && path.totalDistance == 2.0;
    std::cout << (passed ? "✅ PASSED" : "❌ FAILED") << "\n";

    // Test 6: Traffic Manager
    std::cout << "Test 6: Traffic Manager Integration... ";
    TrafficManager tm;
    tm.addJunction(Junction(1, "A", 0, 0, "City", "Area"));
    tm.addJunction(Junction(2, "B", 0, 0, "City", "Area"));
    Road road(1, "AB", 1, 2, 5.0, 60);
    tm.addRoad(road);
    RouteResult route = tm.findRoute(1, 2, true);
    passed = route.found && route.totalDistance == 5.0;
    std::cout << (passed ? "✅ PASSED" : "❌ FAILED") << "\n";

    std::cout << "\n═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    ALL TESTS COMPLETE                          \n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
}

int main(int argc, char* argv[]) {
    // Initialize data
    initializeLahoreData();

    // Parse command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "--server") == 0) {
            // Run API server
            int port = 8080;
            if (argc > 2) {
                port = std::stoi(argv[2]);
            }
            HttpServer server(port, trafficManager);
            if (server.start()) {
                server.run();
            }
            return 0;
        }
        else if (strcmp(argv[1], "--test") == 0) {
            // Run tests
            runTests();
            return 0;
        }
        else if (strcmp(argv[1], "--help") == 0) {
            std::cout << "Smart Traffic Route Optimizer\n\n";
            std::cout << "Usage:\n";
            std::cout << "  traffic_optimizer           Run CLI application\n";
            std::cout << "  traffic_optimizer --server  Run HTTP API server (port 8080)\n";
            std::cout << "  traffic_optimizer --server <port>  Run on specific port\n";
            std::cout << "  traffic_optimizer --test    Run unit tests\n";
            std::cout << "  traffic_optimizer --help    Show this help\n";
            return 0;
        }
    }

    // Run CLI by default
    runCLI();
    return 0;
}
