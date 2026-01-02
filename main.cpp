#include <iostream>
#include <string>
#include <limits>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
#endif

#include "include/TrafficManager.h"
#include "include/HttpServer.h"
#include "include/OSMLoader.h"
#include "include/DataStructureShowcase.h"  // ✅ NEW: Include showcase

using namespace std;

// ============ ICONS ============
#define USE_EMOJIS true

#if USE_EMOJIS
    #define ICON_SUCCESS "✅"
    #define ICON_ERROR "❌"
    #define ICON_WARNING "⚠️"
    #define ICON_INFO "ℹ️"
    #define ICON_SEARCH "🔍"
    #define ICON_MAP "🗺️"
    #define ICON_ROCKET "🚀"
    #define ICON_TRAFFIC "🚦"
    #define ICON_LOADING "⏳"
    #define ICON_FIRE "🔥"
    #define ICON_STATS "📊"
    #define ICON_SPATIAL "🎯"
    #define ICON_AUTO "⚡"
    #define ICON_STRESS "🧪"
    #define ICON_SAVE "💾"
#else
    #define ICON_SUCCESS "[OK]"
    #define ICON_ERROR "[X]"
    #define ICON_WARNING "[!]"
    #define ICON_INFO "[i]"
    #define ICON_SEARCH "[?]"
    #define ICON_MAP "[M]"
    #define ICON_ROCKET "[>>]"
    #define ICON_TRAFFIC "[T]"
    #define ICON_LOADING "[~]"
    #define ICON_FIRE "[F]"
    #define ICON_STATS "[S]"
    #define ICON_SPATIAL "[G]"
    #define ICON_AUTO "[A]"
    #define ICON_STRESS "[T]"
    #define ICON_SAVE "[D]"
#endif

// ============ GLOBAL OBJECTS ============
TrafficManager trafficManager(100);
SpatialIndex spatialIndex;           // ✅ NEW: Spatial search
PerformanceMonitor perfMonitor;       // ✅ NEW: Performance tracking
AutocompleteEngine autocomplete;      // ✅ NEW: Smart autocomplete
StressTester stressTester;            // ✅ NEW: Load testing

// ============ FUNCTION PROTOTYPES ============
void setupWindowsConsole();
void clearScreen();
void printBanner();
void printMenu();
void runCLI();
void loadOSMData();
void initializeLahoreData();

// ✅ NEW: Showcase Features
void showcaseSpatialSearch();
void showcaseAutocomplete();
void showcasePerformanceStats();
void showcaseStressTest();
void showcaseMetricsDashboard();
void showcasePersistence();

// Existing functions
void viewAllJunctions();
void findShortestRoute();
void updateTrafficLevel();
void searchJunction();
void viewRoadNetwork();
void viewStatistics();
void startServer();
void testSmartSearch();
void runTests();

// ============ WINDOWS CONSOLE SETUP ============
void setupWindowsConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

// ============ LOAD DATA ============
void initializeLahoreData() {
    std::cout << "Loading Lahore traffic data...\n";

    // Add Junctions
    Junction j1(1, "Liberty Chowk", 31.5104, 74.3416, "Lahore", "Gulberg");
    Junction j2(2, "Mall Road Chowk", 31.5500, 74.3440, "Lahore", "Mall Road");
    Junction j3(3, "Kalma Chowk", 31.5158, 74.3294, "Lahore", "Gulberg");
    Junction j4(4, "Faisal Chowk", 31.5580, 74.3172, "Lahore", "Faisal Town");
    Junction j5(5, "Thokar Niaz Baig", 31.4711, 74.2675, "Lahore", "Thokar");
    Junction j6(6, "Defence Mor", 31.4795, 74.3848, "Lahore", "DHA");
    
    trafficManager.addJunction(j1);
    trafficManager.addJunction(j2);
    trafficManager.addJunction(j3);
    trafficManager.addJunction(j4);
    trafficManager.addJunction(j5);
    trafficManager.addJunction(j6);

    // ✅ NEW: Add to spatial index and autocomplete
    spatialIndex.addJunction(j1);
    spatialIndex.addJunction(j2);
    spatialIndex.addJunction(j3);
    spatialIndex.addJunction(j4);
    spatialIndex.addJunction(j5);
    spatialIndex.addJunction(j6);
    
    autocomplete.addJunction(j1);
    autocomplete.addJunction(j2);
    autocomplete.addJunction(j3);
    autocomplete.addJunction(j4);
    autocomplete.addJunction(j5);
    autocomplete.addJunction(j6);

    // Add Roads
    Road r1(1, "Main Boulevard Gulberg", 1, 3, 2.5, 50); r1.isTwoWay = true;
    Road r2(2, "Ferozepur Road", 3, 5, 6.0, 60); r2.isTwoWay = true;
    Road r3(3, "Liberty to Defence", 1, 6, 4.5, 40); r3.isTwoWay = true;
    
    trafficManager.addRoad(r1);
    trafficManager.addRoad(r2);
    trafficManager.addRoad(r3);

    std::cout << ICON_SUCCESS << " Loaded " << trafficManager.getJunctionCount() 
              << " junctions and " << trafficManager.getRoadCount() << " roads.\n";
}

void loadOSMData() {
    OSMLoader loader(trafficManager);
    
    std::cout << "\n";
    std::cout << "___________________________________________________________\n";
    std::cout << "|          SMART TRAFFIC ROUTE OPTIMIZER                  |\n";
    std::cout << "|       Real OpenStreetMap Data Integration               |\n";
    std::cout << "|_________________________________________________________|\n";
    
    if (loader.loadJunctions("data/pakistan_osm_junctions.json")) {
        loader.generateRoadNetwork(5.0);
        loader.printStats();
        
        // ✅ NEW: Build spatial index and autocomplete from loaded data
        std::cout << "\n" << ICON_LOADING << " Building advanced indices...\n";
        auto junctions = trafficManager.getAllJunctions();
        for (const auto& j : junctions) {
            spatialIndex.addJunction(j);
            autocomplete.addJunction(j);
        }
        std::cout << ICON_SUCCESS << " Spatial Index & Autocomplete Ready!\n\n";
        
    } else {
        std::cout << "\n" << ICON_WARNING << " OSM file not found. Loading sample data...\n\n";
        initializeLahoreData();
    }
}

// ============ UI FUNCTIONS ============
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printBanner() {
    std::cout << "\n";
    std::cout << "_____________________________________________________________\n";
    std::cout << "|       🚗  SMART TRAFFIC ROUTE OPTIMIZER  🚗              |\n";
    std::cout << "|              Advanced Data Structures Demo                |\n";
    std::cout << "|___________________________________________________________|\n";
    std::cout << "\n";
}

void printMenu() {
    std::cout << "________________________________________________________________\n";
    std::cout << "|                       MAIN MENU                              |\n";
    std::cout << "|______________________________________________________________|\n";
    std::cout << "| BASIC FEATURES                                               |\n";
    std::cout << "|  1. View All Junctions                                       |\n";
    std::cout << "|  2. Find Shortest Route                                      |\n";
    std::cout << "|  3. Update Traffic Level                                     |\n";
    std::cout << "|  4. Search Junction by Name                                  |\n";
    std::cout << "|  5. View Road Network                                        |\n";
    std::cout << "|  6. View System Statistics                                   |\n";
    std::cout << "|______________________________________________________________|\n";
    std::cout << "| " << ICON_FIRE << " ADVANCED SHOWCASE FEATURES (NEW!)                      |\n";
    std::cout << "|  7. " << ICON_SPATIAL << " Spatial Search (Find junctions within radius)        |\n";
    std::cout << "|  8. " << ICON_AUTO << " Smart Autocomplete (B-Tree prefix search)            |\n";
    std::cout << "|  9. " << ICON_STATS << " Performance Dashboard (Live metrics)                 |\n";
    std::cout << "| 10. " << ICON_STRESS << " Stress Test (Simulate 1000 concurrent users)        |\n";
    std::cout << "| 11. " << ICON_SAVE << " Data Persistence (Save/Load to disk)                 |\n";
    std::cout << "|______________________________________________________________|\n";
    std::cout << "| 12. Start API Server                                         |\n";
    std::cout << "|  0. Exit                                                     |\n";
    std::cout << "|______________________________________________________________|\n";
    std::cout << "\nEnter your choice: ";
}

// ============ ✅ NEW SHOWCASE FEATURES ============

void showcaseSpatialSearch() {
    clearScreen();
    printBanner();
    std::cout << "_____________________________________________________________\n";
    std::cout << "|        " << ICON_SPATIAL << " SPATIAL SEARCH DEMO (B-Tree Range Query)             |\n";
    std::cout << "|_____________________________________________________________|\n\n";
    
    std::cout << "This feature uses B-Tree range queries for O(log n + m) search\n";
    std::cout << "where m = number of results.\n\n";
    
    double lat, lng, radius;
    std::cout << "Enter center latitude (e.g., 31.5204 for Lahore): ";
    std::cin >> lat;
    std::cout << "Enter center longitude (e.g., 74.3587): ";
    std::cin >> lng;
    std::cout << "Enter search radius in km (e.g., 5): ";
    std::cin >> radius;
    
    std::cout << "\n" << ICON_LOADING << " Searching...\n\n";
    
    auto results = spatialIndex.findInRadius(lat, lng, radius);
    
    if (results.empty()) {
        std::cout << ICON_ERROR << " No junctions found within " << radius << " km\n";
    } else {
        std::cout << "\n" << ICON_SUCCESS << " Found " << results.size() << " junctions:\n\n";
        std::cout << "________________________________________________________________\n";
        std::cout << "│  #  │         Name          │      Area       │  Distance   │\n";
        std::cout << "|_____|_______________________|_________________|_____________|\n";
        
        int count = 1;
        for (const auto& j : results) {
            // Calculate exact distance
            double dist = spatialIndex.calculateDistance(lat, lng, j.latitude, j.longitude);
            printf("│ %3d │ %-21s │ %-15s │ %7.2f km │\n", 
                   count++, j.name.substr(0, 21).c_str(), 
                   j.area.substr(0, 15).c_str(), dist);
        }
        std::cout << "|_____|_______________________|_________________|_____________|\n";
    }
    
    std::cout << "\n" << ICON_INFO << " This demonstrates B-Tree's efficient range queries!\n";
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void showcaseAutocomplete() {
    clearScreen();
    printBanner();
    std::cout << "_____________________________________________________________\n";
    std::cout << "|       " << ICON_AUTO << " SMART AUTOCOMPLETE (B-Tree Prefix Search)           |\n";
    std::cout << "|_____________________________________________________________|\n\n";
    
    std::cout << "This feature uses B-Tree prefix search for fast autocomplete.\n";
    std::cout << "Try typing partial names like 'lib', 'mall', 'def'\n\n";
    
    std::string prefix;
    std::cout << "Enter search prefix: ";
    std::cin.ignore();
    std::getline(std::cin, prefix);
    
    auto results = autocomplete.search(prefix, 10);
    
    if (results.empty()) {
        std::cout << "\n" << ICON_ERROR << " No matches found for \"" << prefix << "\"\n";
    } else {
        std::cout << "\n" << ICON_SUCCESS << " Top " << results.size() << " suggestions:\n\n";
        std::cout << "________________________________________________________________\n";
        std::cout << "│  #  │           Junction Name           │      City        │\n";
        std::cout << "|_____|___________________________________|__________________|\n";
        
        for (size_t i = 0; i < results.size(); i++) {
            printf("│ %3zu │ %-33s │ %-16s │\n", 
                   i + 1, 
                   results[i].name.substr(0, 33).c_str(),
                   results[i].city.substr(0, 16).c_str());
        }
        std::cout << "|_____|___________________________________|__________________|\n";
    }
    
    std::cout << "\n" << ICON_INFO << " Autocomplete is powered by B-Tree prefix matching!\n";
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void showcasePerformanceStats() {
    clearScreen();
    printBanner();
    
    // First, run some searches to generate data
    std::cout << ICON_LOADING << " Running sample searches to generate metrics...\n\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    trafficManager.findRoute(1, 3, true);
    auto end = std::chrono::high_resolution_clock::now();
    double time1 = std::chrono::duration<double, std::milli>(end - start).count();
    perfMonitor.recordSearch("Dijkstra (Time)", time1);
    
    start = std::chrono::high_resolution_clock::now();
    trafficManager.findRoute(1, 5, false);
    end = std::chrono::high_resolution_clock::now();
    double time2 = std::chrono::duration<double, std::milli>(end - start).count();
    perfMonitor.recordSearch("Dijkstra (Distance)", time2);
    
    start = std::chrono::high_resolution_clock::now();
    spatialIndex.findInRadius(31.5204, 74.3587, 5.0);
    end = std::chrono::high_resolution_clock::now();
    double time3 = std::chrono::duration<double, std::milli>(end - start).count();
    perfMonitor.recordSearch("Spatial Search", time3);
    
    start = std::chrono::high_resolution_clock::now();
    autocomplete.search("lib", 10);
    end = std::chrono::high_resolution_clock::now();
    double time4 = std::chrono::duration<double, std::milli>(end - start).count();
    perfMonitor.recordSearch("Autocomplete", time4);
    
    std::cout << ICON_SUCCESS << " Sample searches complete!\n\n";
    
    // Show statistics
    perfMonitor.showStats();
    
    std::cout << "\n" << ICON_INFO << " Performance monitoring tracks all search operations!\n";
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// void showcaseStressTest() {
//     clearScreen();
//     printBanner();
//     std::cout << "_____________________________________________________________\n";
//     std::cout << "|        " << ICON_STRESS << " STRESS TEST (Concurrent Load Testing)              |\n";
//     std::cout << "|_____________________________________________________________|\n\n";
    
//     std::cout << "⚠️  Note: Stress test temporarily disabled in this build.\n";
//     std::cout << "   Use API endpoints for performance testing.\n\n";
    
//     std::cout << "\nPress Enter to continue...";
//     std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//     std::cin.get();
// }

// void showcaseMetricsDashboard() {
//     clearScreen();
//     printBanner();
//     std::cout << "_____________________________________________________________\n";
//     std::cout << "|        " << ICON_STATS << " METRICS DASHBOARD                               |\n";
//     std::cout << "|_____________________________________________________________|\n\n";
    
//     std::cout << "📊 System Statistics:\n\n";
//     std::cout << "  Junctions: " << trafficManager.getJunctionCount() << "\n";
//     std::cout << "  Roads: " << trafficManager.getRoadCount() << "\n";
//     std::cout << "  Cache Hit Rate: " << trafficManager.getCacheHitRate() << "%\n\n";
    
//     std::cout << "ℹ️  For detailed metrics, use the API endpoint:\n";
//     std::cout << "   GET http://localhost:8080/api/metrics\n\n";
    
//     std::cout << "\nPress Enter to continue...";
//     std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//     std::cin.get();
// }

// void showcasePersistence() {
//     clearScreen();
//     printBanner();
//     std::cout << "_____________________________________________________________\n";
//     std::cout << "|         " << ICON_SAVE << " DATA PERSISTENCE                                |\n";
//     std::cout << "|_____________________________________________________________|\n\n";
    
//     std::cout << "⚠️  Note: Direct persistence temporarily disabled.\n";
//     std::cout << "   Data is automatically cached in memory during runtime.\n\n";
    
//     std::cout << "ℹ️  For production deployment, enable database integration.\n\n";
    
//     std::cout << "\nPress Enter to continue...";
//     std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//     std::cin.get();
// }

// ============ EXISTING FUNCTIONS (Keep as-is) ============
void viewAllJunctions() {
    clearScreen();
    printBanner();
    std::cout << "_____________________________________________________________\n";
    std::cout << "                      ALL JUNCTIONS                          \n";
    std::cout << "_____________________________________________________________\n\n";

    auto junctions = trafficManager.getAllJunctions();
    
    std::cout << "____________________________________________________________________\n";
    std::cout << "│  ID  │           Name            │      Area       │    City     │\n";
    std::cout << "|______|___________________________|_________________|_____________|\n";
    
    for (const auto& j : junctions) {
        printf("│ %4d │ %-25s │ %-15s │ %-11s │\n", 
               j.id, j.name.substr(0, 25).c_str(), 
               j.area.substr(0, 15).c_str(), j.city.substr(0, 11).c_str());
    }
    
    std::cout << "|______|___________________________|_________________|_____________|\n";
    std::cout << "\nTotal Junctions: " << junctions.size() << "\n";
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void findShortestRoute() {
    clearScreen();
    printBanner();
    std::cout << "_____________________________________________________________\n";
    std::cout << "                    FIND SHORTEST ROUTE                      \n";
    std::cout << "_____________________________________________________________\n\n";

    auto junctions = trafficManager.getAllJunctions();
    std::cout << "Available Junctions:\n";
    for (size_t i = 0; i < std::min(size_t(10), junctions.size()); i++) {
        std::cout << "  " << junctions[i].id << ". " << junctions[i].name << "\n";
    }
    if (junctions.size() > 10) {
        std::cout << "  ... and " << (junctions.size() - 10) << " more\n";
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
    
    std::cout << "\n" << ICON_SEARCH << " Calculating route...\n\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    RouteResult result = trafficManager.findRoute(fromId, toId, useTime);
    auto end = std::chrono::high_resolution_clock::now();
    double searchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    // ✅ Record performance
    perfMonitor.recordSearch(useTime ? "Route (Time)" : "Route (Distance)", searchTime);

    if (result.found) {
        std::cout << "_____________________________________________________________\n";
        std::cout << "|                    " << ICON_SUCCESS << " ROUTE FOUND!                         |\n";
        std::cout << "|____________________________________________________________|\n\n";

        std::cout << "📏 Total Distance: " << result.totalDistance << " km\n";
        std::cout << "⏱️  Search Time: " << searchTime << " ms\n";
        
        int hours = static_cast<int>(result.totalTime / 60);
        int minutes = static_cast<int>(result.totalTime) % 60;
        std::cout << "⏱️  Estimated Time: ";
        if (hours > 0) std::cout << hours << "h ";
        std::cout << minutes << " minutes\n\n";

        std::cout << "🗺️ Route Path:\n";
        std::cout << "_____________________________________________________________\n";
        
        for (size_t i = 0; i < result.junctions.size(); ++i) {
            std::cout << "│  " << (i + 1) << ". " << result.junctions[i].name;
            std::cout << " (" << result.junctions[i].area << ")\n";
            if (i < result.junctions.size() - 1) {
                std::cout << "│       ↓\n";
            }
        }
        std::cout << "|_____________________________________________________________|\n";
    } else {
        std::cout << ICON_ERROR << " No route found!\n";
    }

    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void updateTrafficLevel() {
    clearScreen();
    printBanner();
    std::cout << "_____________________________________________________________\n";
    std::cout << "                     UPDATE TRAFFIC LEVEL                    \n";
    std::cout << "_____________________________________________________________\n\n";

    auto roads = trafficManager.getAllRoads();
    std::cout << "Available Roads:\n";
    std::cout << "_______________________________________________________________\n";
    std::cout << "|  ID  |         Road Name         |      Current Traffic     |\n";
    std::cout << "|______|___________________________|__________________________|\n";
    
    for (const auto& r : roads) {
        printf("│ %4d │ %-25s │ %-24s │\n", 
               r.id, r.name.substr(0, 25).c_str(), 
               trafficLevelToString(r.trafficLevel).c_str());
    }
    std::cout << "|______|___________________________|__________________________|\n\n";

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
    std::cout << "_____________________________________________________________\n";
    std::cout << "                     SEARCH JUNCTION                         \n";
    std::cout << "_____________________________________________________________\n\n";

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
            std::cout << "_____________________________________________________________\n";
            std::cout << "| ID: " << j.id << "\n";
            std::cout << "| Name: " << j.name << "\n";
            std::cout << "| Area: " << j.area << ", " << j.city << "\n";
            std::cout << "| Coordinates: " << j.latitude << ", " << j.longitude << "\n";
            std::cout << "|_____________________________________________________________|\n";
        }
    }

    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

// Replace these functions in main.cpp:

void showcaseStressTest() {
    clearScreen();
    printBanner();
    std::cout << "_____________________________________________________________\n";
    std::cout << "|        🧪 STRESS TEST (Concurrent Load Testing)              |\n";
    std::cout << "|_____________________________________________________________|\n\n";
    
    std::cout << "⚡ Simulating 100 concurrent users...\n";
    std::cout << "   Each user performs 50 random searches\n\n";
    
    // Get references to data structures (you'll need to expose these)
    auto junctions = trafficManager.getAllJunctions();

    if (junctions.empty()) {
        std::cout << "❌ No junctions loaded!\n";
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    // Create temp B-Tree and Hash Table for testing
    BTree<int, Junction> testBTree(5);
    HashTable<int, Junction> testHashTable(1024);
    
    std::cout << "📦 Loading " << junctions.size() << " junctions into test structures...\n";

    // Load data
    for (const auto& j : junctions) {
        testBTree.insert(j.id, j);
        testHashTable.insert(j.id, j);
    }
    
    std::cout << "✅ Data loaded successfully!\n\n";
    std::cout << "🏃 Starting stress test...\n\n";

    auto start = std::chrono::high_resolution_clock::now();

    std::atomic<int> btreeSuccess(0);
    std::atomic<int> hashSuccess(0);
    std::atomic<int> totalQueries(0);

    const int userCount = 100;
    const int queriesPerUser = 50;
    std::vector<std::thread> threads;

    for (int i = 0; i < userCount; i++) {
        threads.emplace_back([&, i]() {
            std::mt19937 rng(i * 1000);
            std::uniform_int_distribution<> dist(0, junctions.size() - 1);
            
            for (int q = 0; q < queriesPerUser; q++) {
                int randomIndex = dist(rng);
                int randomId = junctions[randomIndex].id;
                
                Junction result;
                
                // Alternate between B-Tree and Hash Table
                if (q % 2 == 0) {
                    if (testBTree.search(randomId, &result)) {
                        btreeSuccess++;
                    }
                } else {
                    if (testHashTable.search(randomId, &result)) {
                        hashSuccess++;
                    }
                }
                
                totalQueries++;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    int total = userCount * queriesPerUser;
    double queriesPerSec = (total * 1000.0) / duration.count();
    double avgLatency = (double)duration.count() / total;

    // Display results
    std::cout << "╔═════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    📊 TEST RESULTS                          ║\n";
    std::cout << "╚═════════════════════════════════════════════════════════════╝\n\n";

    std::cout << "  ✅ Test Completed Successfully!\n\n";

    std::cout << "  📈 PERFORMANCE METRICS:\n";
    std::cout << "  ├─ Total Users:       " << userCount << "\n";
    std::cout << "  ├─ Queries per User:  " << queriesPerUser << "\n";
    std::cout << "  ├─ Total Queries:     " << total << "\n";
    std::cout << "  ├─ Duration:          " << duration.count() << " ms\n";
    std::cout << "  ├─ Throughput:        " << (int)queriesPerSec << " queries/sec\n";
    std::cout << "  └─ Avg Latency:       " << avgLatency << " ms/query\n\n";

    std::cout << "  🎯 SUCCESS RATE:\n";
    std::cout << "  ├─ B-Tree:            " << btreeSuccess << "/" << (total/2) << " searches\n";
    std::cout << "  └─ Hash Table:        " << hashSuccess << "/" << (total/2) << " searches\n\n";

    td::cout << "  💡 ANALYSIS:\n";
    if (queriesPerSec > 10000) {
        std::cout << "  ✅ EXCELLENT: System handles 10K+ queries/sec!\n";
    } else if (queriesPerSec > 5000) {
        std::cout << "  ✅ GOOD: System handles 5K+ queries/sec\n";
    } else {
        std::cout << "  ⚠️  MODERATE: System handles " << (int)queriesPerSec << " queries/sec\n";
    }

    std::cout << "\n  🔍 This demonstrates thread-safe concurrent access to\n";
    std::cout << "     B-Tree (O(log n)) and Hash Table (O(1)) structures!\n";

    std::cout << "\n╚═════════════════════════════════════════════════════════════╝\n";

    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    // // Run stress test
    // StressTester tester;
    // tester.simulateConcurrentUsers(testBTree, testHashTable, 100, 50);
    
    // std::cout << "\n✅ Stress test completed!\n";
    // std::cout << "\nPress Enter to continue...";
    // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // std::cin.get();
}

void showcasePersistence() {
    clearScreen();
    printBanner();
    std::cout << "_____________________________________________________________\n";
    std::cout << "|         💾 DATA PERSISTENCE                                |\n";
    std::cout << "|_____________________________________________________________|\n\n";
    
    // Create a test B-Tree
    BTree<int, Junction> testBTree(5);
    
    auto junctions = trafficManager.getAllJunctions();
    for (const auto& j : junctions) {
        testBTree.insert(j.id, j);
    }
    
    std::cout << "Current data: " << junctions.size() << " junctions\n\n";
    
    // Save to disk
    std::cout << "💾 Saving to disk...\n";
    if (PersistenceEngine::saveBTree(testBTree, "data/junctions_backup.dat")) {
        std::cout << "✅ Save successful!\n\n";
        
        // Clear and reload
        std::cout << "📁 Clearing memory and reloading...\n";
        BTree<int, Junction> newBTree(5);
        
        if (PersistenceEngine::loadBTree(newBTree, "data/junctions_backup.dat")) {
            std::cout << "✅ Load successful!\n";
            std::cout << "   Loaded " << newBTree.size() << " junctions\n";
        }
    } else {
        std::cout << "❌ Save failed!\n";
    }
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void showcaseMetricsDashboard() 
{
    // Get all junctions to populate test structures
    auto junctions = trafficManager.getAllJunctions();

    // Create test structures
    BTree<int, Junction> testBTree(5);
    HashTable<int, Junction> testHashTable(1024);

    std::cout << "📦 Analyzing data structures...\n\n";

    for (const auto& j : junctions) {
        testBTree.insert(j.id, j);
        testHashTable.insert(j.id, j);
    }

    // Get metrics
    auto btreeMetrics = testBTree.getMetrics();
    auto hashMetrics = testHashTable.getMetrics();

    std::cout << "╔═════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              🌲 B-TREE METRICS                              ║\n";
    std::cout << "╚═════════════════════════════════════════════════════════════╝\n\n";

    std::cout << "  📊 STRUCTURE:\n";
    std::cout << "  ├─ Height:            " << btreeMetrics.height << " levels\n";
    std::cout << "  ├─ Nodes:             " << btreeMetrics.nodeCount << "\n";
    std::cout << "  ├─ Elements:          " << btreeMetrics.elementCount << "\n";
    std::cout << "  └─ Keys/Node (avg):   " << std::fixed << std::setprecision(1) 
            << btreeMetrics.avgKeysPerNode << "\n\n";

    std::cout << "  ⚡ PERFORMANCE:\n";
    std::cout << "  ├─ Search:            O(log n) = ~" << btreeMetrics.height << " comparisons\n";
    std::cout << "  ├─ Insert:            O(log n)\n";
    std::cout << "  ├─ Delete:            O(log n)\n";
    std::cout << "  └─ Range Query:       O(log n + m)\n\n";

    std::cout << "  💾 MEMORY:\n";
    std::cout << "  └─ Total Usage:       " << (btreeMetrics.memoryBytes / 1024) << " KB\n\n";

    std::cout << "╔═════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              # HASH TABLE METRICS                           ║\n";
    std::cout << "╚═════════════════════════════════════════════════════════════╝\n\n";

    std::cout << "  📊 STRUCTURE:\n";
    std::cout << "  ├─ Buckets:           " << hashMetrics.bucketCount << "\n";
    std::cout << "  ├─ Elements:          " << hashMetrics.elementCount << "\n";
    std::cout << "  ├─ Load Factor:       " << std::fixed << std::setprecision(2) 
            << hashMetrics.loadFactor << " (target: 0.75)\n";
    std::cout << "  ├─ Longest Chain:     " << hashMetrics.longestChain << "\n";
    std::cout << "  ├─ Avg Chain:         " << std::fixed << std::setprecision(2) 
            << hashMetrics.avgChainLength << "\n";
    std::cout << "  └─ Collisions:        " << hashMetrics.collisions << "\n\n";

    std::cout << "  ⚡ PERFORMANCE:\n";
    std::cout << "  ├─ Search:            O(1) average = ~" << (int)hashMetrics.avgChainLength << " comparisons\n";
    std::cout << "  ├─ Insert:            O(1) average\n";
    std::cout << "  └─ Delete:            O(1) average\n\n";

    std::cout << "  💾 MEMORY:\n";
    std::cout << "  └─ Total Usage:       " << (hashMetrics.memoryUsageBytes / 1024) << " KB\n\n";

    std::cout << "╔═════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              📈 COMPARISON & ANALYSIS                       ║\n";
    std::cout << "╚═════════════════════════════════════════════════════════════╝\n\n";

    std::cout << "  🎯 WHEN TO USE EACH:\n\n";

    std::cout << "  🌲 B-TREE ADVANTAGES:\n";
    std::cout << "  ├─ ✅ Sorted traversal\n";
    std::cout << "  ├─ ✅ Range queries\n";
    std::cout << "  ├─ ✅ Memory efficient\n";
    std::cout << "  └─ ✅ Better cache locality\n\n";

    std::cout << "  # HASH TABLE ADVANTAGES:\n";
    std::cout << "  ├─ ✅ Faster single lookups\n";
    std::cout << "  ├─ ✅ Simpler implementation\n";
    std::cout << "  └─ ✅ O(1) average case\n\n";

    std::cout << "  💡 THIS PROJECT USES BOTH:\n";
    std::cout << "  ├─ B-Tree → Name-based search\n";
    std::cout << "  └─ Hash Table → ID-based lookup\n\n";

    // Show efficiency comparison
    double btreeEfficiency = (1.0 / btreeMetrics.height) * 100;
    double hashEfficiency = (1.0 / hashMetrics.avgChainLength) * 100;

    std::cout << "  📊 RELATIVE EFFICIENCY:\n";
    std::cout << "  ├─ B-Tree:   " << std::fixed << std::setprecision(1) << btreeEfficiency << "%\n";
    std::cout << "  └─ Hash:     " << std::fixed << std::setprecision(1) << hashEfficiency << "%\n";

    std::cout << "\n╚═════════════════════════════════════════════════════════════╝\n";

    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}


void viewRoadNetwork() {
    clearScreen();
    printBanner();
    std::cout << "_____________________________________________________________\n";
    std::cout << "                     ROAD NETWORK                            \n";
    std::cout << "_____________________________________________________________\n\n";

    auto roads = trafficManager.getAllRoads();
    
    for (const auto& r : roads) {
        Junction source, dest;
        trafficManager.getJunction(r.sourceJunction, &source);
        trafficManager.getJunction(r.destJunction, &dest);
        
        std::cout << "_____________________________________________________________\n";
        std::cout << "| 🛣️  " << r.name << "\n";
        std::cout << "| " << source.name << " ↔️ " << dest.name << "\n";
        std::cout << "| Distance: " << r.distance << " km | Speed Limit: " << r.speedLimit << " km/h\n";
        std::cout << "| Traffic: " << trafficLevelToString(r.trafficLevel);
        std::cout << " (×" << getTrafficMultiplier(r.trafficLevel) << ")\n";
        std::cout << "| Est. Time: " << r.getActualTime() << " minutes\n";
        std::cout << "|_____________________________________________________________|\n";
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
    std::cout << "  |_ B-Tree: Junction name indexing (O(log n) search)\n";
    std::cout << "  |_ Hash Table: Junction ID lookup (O(1) average)\n";
    std::cout << "  |_ Graph: Road network with weighted edges\n";
    std::cout << "  |_ Min-Heap: Dijkstra's algorithm optimization\n";
    std::cout << "  |_ LRU Cache: Route result caching\n";
    
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// Smart Search Test Function
void testSmartSearch() {
    clearScreen();
    printBanner();
    std::cout << "_____________________________________________________________\n";
    std::cout << "               🌍 SMART SEARCH (OSM)                         \n";
    std::cout << "_____________________________________________________________\n\n";

    std::string query;
    std::cout << "Enter location name: ";
    std::cin.ignore();
    std::getline(std::cin, query);

    std::cout << "\n🔍 Searching with intelligent matching...\n\n";

    // Try fuzzy search
    auto results = trafficManager.intelligentSearch(query);  // 60% similarity threshold

    if (results.empty()) {
        std::cout << "❌ No matches found for \"" << query << "\"\n\n";
        std::cout << "💡 Tips:\n";
        std::cout << "   • Try shorter names (e.g., 'Liberty' instead of 'Liberty Chowk Lahore')\n";
        std::cout << "   • Check spelling\n";
        std::cout << "   • Try partial matches\n";
    } else {
        std::cout << "✅ Found " << results.size() << " match(es):\n\n";
        
        for (const auto& j : results) {
            std::cout << "_____________________________________________________________\n";
            std::cout << "| 📍 " << j.name << "\n";
            std::cout << "| ID: " << j.id << "\n";
            std::cout << "| Location: " << j.area << ", " << j.city << "\n";
            std::cout << "| Coordinates: " << j.latitude << ", " << j.longitude << "\n";
            std::cout << "|____________________________________________________________|\n";
        }
    }

    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void startServer() {
    clearScreen();
    printBanner();
    
    int port = 8080;
    std::cout << "Starting HTTP API Server on port " << port << "...\n\n";
    
    HttpServer server(port, trafficManager);
    
    if (server.start()) {
        std::cout << "✅ Server started successfully!\n";
        std::cout << "🌐 Open browser: http://localhost:8080/api/health\n\n";
        std::cout << "⌨️  Press Ctrl+C to stop (or close this window)\n\n";
        server.run();
    } else {
        std::cout << "❌ Failed to start server.\n";
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
}

// void printMenu() {
//     std::cout << "__________________________________________\n";
//     std::cout << "|             MAIN MENU                  |\n";
//     std::cout << "|________________________________________|\n";
//     std::cout << "|  1. View All Junctions                 |\n";
//     std::cout << "|  2. Find Shortest Route                |\n";
//     std::cout << "|  3. Update Traffic Level               |\n";
//     std::cout << "|  4. Search Junction by Name            |\n";
//     std::cout << "|  5. View Road Network                  |\n";
//     std::cout << "|  6. View System Statistics             |\n";
//     std::cout << "|  7. Smart Search (Fuzzy) 🌍 NEW       |\n";
//     std::cout << "|  8. Start API Server                   |\n";
//     std::cout << "|  0. Exit                               |\n";
//     std::cout << "|________________________________________|\n";
//     std::cout << "\nEnter your choice: ";
// }

// Keep other existing functions (updateTrafficLevel, searchJunction, etc.) as-is...

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
            
            // ✅ NEW: Showcase features
            case 7: showcaseSpatialSearch(); break;
            case 8: showcaseAutocomplete(); break;
            case 9: showcasePerformanceStats(); break;
            case 10: showcaseStressTest(); break;
            case 11: showcasePersistence(); break;
            
            case 12: startServer(); break;  // ✅ FIXED!
            case 0:
                clearScreen();
                printBanner();
                std::cout << "Thank you for using Smart Traffic Route Optimizer!\n";
                std::cout << "Goodbye! 👋\n\n";
                break;
            default:
                std::cout << "\n" << ICON_WARNING << " Invalid choice!\n";
                std::cout << "Press Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
        }
    } while (choice != 0);
}

int main(int argc, char* argv[]) {
    setupWindowsConsole();
    loadOSMData();
    runCLI();
    return 0;
}
