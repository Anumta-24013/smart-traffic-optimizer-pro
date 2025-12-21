# Smart Traffic Route Optimizer 🚗

A DSA-based traffic route optimization system for Pakistan cities, built with C++17.

## 📋 Features

- **B-Tree Indexing** - O(log n) junction name search
- **Hash Table** - O(1) junction ID lookup
- **Graph + Dijkstra** - Shortest path calculation with traffic multipliers
- **Min-Heap Priority Queue** - Optimized Dijkstra (O((V+E) log V))
- **LRU Cache** - Route result caching for performance
- **REST API Server** - HTTP endpoints for web integration
- **Web Frontend** - Dark theme UI with glassmorphism design
- **CLI Application** - Interactive console interface

## 🏗️ Project Structure

```
smart-traffic-optimizer/
├── include/
│   ├── BTree.h          # B-Tree data structure
│   ├── HashTable.h      # Hash table implementation
│   ├── MinHeap.h        # Min-heap priority queue
│   ├── Graph.h          # Graph with Dijkstra's algorithm
│   ├── LRUCache.h       # LRU cache implementation
│   ├── Models.h         # Junction, Road, User data models
│   ├── TrafficManager.h # Core system logic
│   └── HttpServer.h     # REST API server
├── data/
│   └── lahore_data.json # Sample data for Lahore
├── frontend/
│   └── index.html       # Web interface
├── main.cpp             # Entry point
├── CMakeLists.txt       # CMake build configuration
├── build.bat            # Windows build script
└── run.bat              # Windows run script
```

## 🚀 Quick Start

### Prerequisites

- **Windows**: MinGW-w64 or Visual Studio with C++ support
- **Linux/Mac**: GCC 7+ or Clang 5+

### Build & Run (Windows)

```batch
# Build the project
build.bat

# Run CLI application
run.bat

# Run API server
run.bat --server

# Run tests
run.bat --test
```

### Build & Run (Linux/Mac)

```bash
# Build with CMake
mkdir build && cd build
cmake ..
make

# Or build directly with g++
g++ -std=c++17 -O2 -o traffic_optimizer main.cpp -pthread

# Run CLI
./traffic_optimizer

# Run API server
./traffic_optimizer --server

# Run tests
./traffic_optimizer --test
```

## 📡 API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/health` | Health check |
| GET | `/api/junctions` | Get all junctions |
| GET | `/api/junction?id=1` | Get junction by ID |
| GET | `/api/roads` | Get all roads |
| GET | `/api/route?from=1&to=5` | Find shortest route |
| GET | `/api/traffic` | Get traffic levels |
| POST | `/api/traffic?road=1&level=3` | Update traffic level |
| GET | `/api/search?q=liberty` | Search junctions |
| GET | `/api/stats` | System statistics |

## 🗺️ Sample Data (Lahore)

The system comes preloaded with 12 junctions and 15 roads from Lahore:

- Liberty Chowk
- Mall Road Chowk
- Kalma Chowk
- Faisal Chowk
- Thokar Niaz Baig
- Defence Mor
- Model Town Link Road
- Barkat Market
- Chungi Amar Sidhu
- Data Darbar
- Lahore Railway Station
- Gaddafi Stadium

## 📊 Data Structures

### B-Tree (Junction Indexing)
- Stores junction names for efficient search
- Time Complexity: O(log n) for all operations
- Self-balancing for optimal performance

### Hash Table (ID Lookup)
- O(1) average case for junction retrieval
- Chaining for collision resolution
- Dynamic resizing when load factor exceeds threshold

### Min-Heap (Dijkstra Optimization)
- Used in Dijkstra's algorithm for priority queue
- Supports decrease-key operation for A* algorithm
- Time Complexity: O(log n) for insert/extract

### LRU Cache (Route Caching)
- Caches recently calculated routes
- Doubly-linked list + HashMap implementation
- O(1) for get and put operations

### Graph (Road Network)
- Adjacency list representation
- Weighted edges with traffic multipliers
- Supports both directed and undirected edges

## 🎯 CLI Menu Options

1. **View All Junctions** - Display all available junctions
2. **Find Shortest Route** - Calculate optimal path between two points
3. **Update Traffic Level** - Modify traffic conditions on roads
4. **Search Junction** - Find junctions by partial name
5. **View Road Network** - Display all roads with details
6. **View Statistics** - System performance metrics
7. **Start API Server** - Launch HTTP REST API

## 🌐 Web Frontend

Open `frontend/index.html` in a browser (with the API server running) for a modern web interface featuring:

- Dark theme with glassmorphism design
- Interactive route finder
- Real-time junction list
- Traffic status display
- Route visualization

## 📈 Traffic Levels

| Level | Multiplier | Description |
|-------|------------|-------------|
| Low | 0.8x | Free flowing |
| Normal | 1.0x | Standard traffic |
| Heavy | 1.5x | Congested |
| Severe | 2.5x | Gridlock |

## 🧪 Testing

Run the test suite:
```bash
./traffic_optimizer --test
```

Tests include:
- B-Tree operations
- Hash Table operations
- Min-Heap operations
- LRU Cache operations
- Dijkstra's algorithm
- Traffic Manager integration

## 📝 License

This project is created for educational purposes as a DSA course project.

## 👤 Author

Smart Traffic Route Optimizer - Pakistan Cities
