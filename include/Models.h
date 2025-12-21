/**
 * Smart Traffic Route Optimizer
 * Junction and Road Data Models
 */

#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Traffic level enumeration
enum class TrafficLevel {
    LOW = 1,      // Green - Free flowing
    NORMAL = 2,   // Yellow - Normal traffic
    HEAVY = 3,    // Orange - Heavy traffic
    SEVERE = 4    // Red - Severe congestion
};

// Convert traffic level to multiplier
inline double getTrafficMultiplier(TrafficLevel level) {
    switch (level) {
        case TrafficLevel::LOW: return 0.8;
        case TrafficLevel::NORMAL: return 1.0;
        case TrafficLevel::HEAVY: return 1.5;
        case TrafficLevel::SEVERE: return 2.5;
        default: return 1.0;
    }
}

// Convert traffic level to string
inline std::string trafficLevelToString(TrafficLevel level) {
    switch (level) {
        case TrafficLevel::LOW: return "Low";
        case TrafficLevel::NORMAL: return "Normal";
        case TrafficLevel::HEAVY: return "Heavy";
        case TrafficLevel::SEVERE: return "Severe";
        default: return "Unknown";
    }
}

// Junction (Intersection) data structure
struct Junction {
    int id;
    std::string name;
    double latitude;
    double longitude;
    std::string city;
    std::string area;
    bool hasTrafficSignal;
    std::vector<int> connectedJunctions;

    Junction() : id(0), latitude(0), longitude(0), hasTrafficSignal(false) {}

    Junction(int _id, const std::string& _name, double _lat, double _lng,
             const std::string& _city = "", const std::string& _area = "")
        : id(_id), name(_name), latitude(_lat), longitude(_lng),
          city(_city), area(_area), hasTrafficSignal(true) {}

    // Calculate Haversine distance to another junction (in km)
    double distanceTo(const Junction& other) const {
        const double R = 6371.0; // Earth's radius in km
        double lat1 = latitude * M_PI / 180.0;
        double lat2 = other.latitude * M_PI / 180.0;
        double dLat = (other.latitude - latitude) * M_PI / 180.0;
        double dLon = (other.longitude - longitude) * M_PI / 180.0;

        double a = sin(dLat/2) * sin(dLat/2) +
                   cos(lat1) * cos(lat2) * sin(dLon/2) * sin(dLon/2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));

        return R * c;
    }

    // Convert to JSON string
    std::string toJson() const {
        std::string json = "{";
        json += "\"id\":" + std::to_string(id) + ",";
        json += "\"name\":\"" + name + "\",";
        json += "\"latitude\":" + std::to_string(latitude) + ",";
        json += "\"longitude\":" + std::to_string(longitude) + ",";
        json += "\"city\":\"" + city + "\",";
        json += "\"area\":\"" + area + "\",";
        json += "\"hasTrafficSignal\":" + std::string(hasTrafficSignal ? "true" : "false");
        json += "}";
        return json;
    }
};

// Road (Edge) data structure
struct Road {
    int id;
    std::string name;
    int sourceJunction;
    int destJunction;
    double distance;      // in kilometers
    double speedLimit;    // in km/h
    double baseTime;      // base travel time in minutes
    TrafficLevel trafficLevel;
    bool isTwoWay;
    std::string roadType; // highway, main, local, etc.

    Road() : id(0), sourceJunction(0), destJunction(0), distance(0),
             speedLimit(40), baseTime(0), trafficLevel(TrafficLevel::NORMAL),
             isTwoWay(true), roadType("main") {}

    Road(int _id, const std::string& _name, int _src, int _dest, 
         double _dist, double _speed = 40.0)
        : id(_id), name(_name), sourceJunction(_src), destJunction(_dest),
          distance(_dist), speedLimit(_speed), trafficLevel(TrafficLevel::NORMAL),
          isTwoWay(true), roadType("main") {
        // Calculate base time: time = distance / speed (in hours) * 60 (to minutes)
        baseTime = (distance / speedLimit) * 60.0;
    }

    // Get actual travel time considering traffic
    double getActualTime() const {
        return baseTime * getTrafficMultiplier(trafficLevel);
    }

    // Update traffic level
    void setTrafficLevel(TrafficLevel level) {
        trafficLevel = level;
    }

    // Convert to JSON string
    std::string toJson() const {
        std::string json = "{";
        json += "\"id\":" + std::to_string(id) + ",";
        json += "\"name\":\"" + name + "\",";
        json += "\"source\":" + std::to_string(sourceJunction) + ",";
        json += "\"destination\":" + std::to_string(destJunction) + ",";
        json += "\"distance\":" + std::to_string(distance) + ",";
        json += "\"speedLimit\":" + std::to_string(speedLimit) + ",";
        json += "\"baseTime\":" + std::to_string(baseTime) + ",";
        json += "\"actualTime\":" + std::to_string(getActualTime()) + ",";
        json += "\"trafficLevel\":\"" + trafficLevelToString(trafficLevel) + "\",";
        json += "\"trafficMultiplier\":" + std::to_string(getTrafficMultiplier(trafficLevel)) + ",";
        json += "\"isTwoWay\":" + std::string(isTwoWay ? "true" : "false") + ",";
        json += "\"roadType\":\"" + roadType + "\"";
        json += "}";
        return json;
    }
};

// Route result structure
struct RouteResult {
    std::vector<Junction> junctions;
    std::vector<Road> roads;
    double totalDistance;
    double totalTime;
    bool found;

    RouteResult() : totalDistance(0), totalTime(0), found(false) {}

    std::string toJson() const {
        std::string json = "{";
        json += "\"found\":" + std::string(found ? "true" : "false") + ",";
        json += "\"totalDistance\":" + std::to_string(totalDistance) + ",";
        json += "\"totalTime\":" + std::to_string(totalTime) + ",";
        
        json += "\"junctions\":[";
        for (size_t i = 0; i < junctions.size(); ++i) {
            json += junctions[i].toJson();
            if (i < junctions.size() - 1) json += ",";
        }
        json += "],";
        
        json += "\"path\":[";
        for (size_t i = 0; i < junctions.size(); ++i) {
            json += std::to_string(junctions[i].id);
            if (i < junctions.size() - 1) json += ",";
        }
        json += "]";
        
        json += "}";
        return json;
    }
};

// User data structure (for authentication system)
struct User {
    int id;
    std::string username;
    std::string email;
    std::string passwordHash;
    std::string salt;
    std::vector<std::string> favoriteRoutes;
    std::vector<std::string> searchHistory;
    bool isAdmin;
    long long createdAt;
    long long lastLogin;

    User() : id(0), isAdmin(false), createdAt(0), lastLogin(0) {}

    User(int _id, const std::string& _username, const std::string& _email)
        : id(_id), username(_username), email(_email), isAdmin(false),
          createdAt(0), lastLogin(0) {}

    std::string toJson() const {
        std::string json = "{";
        json += "\"id\":" + std::to_string(id) + ",";
        json += "\"username\":\"" + username + "\",";
        json += "\"email\":\"" + email + "\",";
        json += "\"isAdmin\":" + std::string(isAdmin ? "true" : "false");
        json += "}";
        return json;
    }
};

// Session data structure
struct Session {
    std::string token;
    int userId;
    long long createdAt;
    long long expiresAt;
    std::string ipAddress;

    Session() : userId(0), createdAt(0), expiresAt(0) {}

    bool isExpired() const {
        // Simple check - in real impl, compare with current time
        return false; // Placeholder
    }
};

#endif // MODELS_H
