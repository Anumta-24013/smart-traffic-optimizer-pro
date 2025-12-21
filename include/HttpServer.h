/**
 * Smart Traffic Route Optimizer
 * HTTP Server Implementation
 * 
 * Simple REST API server for traffic management
 * Endpoints: /api/health, /api/junctions, /api/route, /api/traffic
 */

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <functional>
#include <map>
#include <regex>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#include "TrafficManager.h"

class HttpServer {
private:
    int port;
    SOCKET serverSocket;
    bool running;
    TrafficManager& trafficManager;
    std::map<std::string, std::function<std::string(const std::string&, const std::string&)>> routes;

    // Parse HTTP request
    struct HttpRequest {
        std::string method;
        std::string path;
        std::string query;
        std::string body;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> params;
    };

    HttpRequest parseRequest(const std::string& raw) {
        HttpRequest req;
        std::istringstream stream(raw);
        std::string line;

        // Parse request line
        if (std::getline(stream, line)) {
            std::istringstream lineStream(line);
            lineStream >> req.method >> req.path;
            
            // Extract query string
            size_t queryPos = req.path.find('?');
            if (queryPos != std::string::npos) {
                req.query = req.path.substr(queryPos + 1);
                req.path = req.path.substr(0, queryPos);
                
                // Parse query parameters
                std::istringstream queryStream(req.query);
                std::string param;
                while (std::getline(queryStream, param, '&')) {
                    size_t eqPos = param.find('=');
                    if (eqPos != std::string::npos) {
                        req.params[param.substr(0, eqPos)] = param.substr(eqPos + 1);
                    }
                }
            }
        }

        // Parse headers
        while (std::getline(stream, line) && line != "\r" && !line.empty()) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 2);
                if (!value.empty() && value.back() == '\r') {
                    value.pop_back();
                }
                req.headers[key] = value;
            }
        }

        // Parse body
        std::ostringstream bodyStream;
        while (std::getline(stream, line)) {
            bodyStream << line;
        }
        req.body = bodyStream.str();

        return req;
    }

    // Create HTTP response
    std::string createResponse(int statusCode, const std::string& body, 
                               const std::string& contentType = "application/json") {
        std::string statusText;
        switch (statusCode) {
            case 200: statusText = "OK"; break;
            case 201: statusText = "Created"; break;
            case 400: statusText = "Bad Request"; break;
            case 404: statusText = "Not Found"; break;
            case 500: statusText = "Internal Server Error"; break;
            default: statusText = "Unknown";
        }

        std::ostringstream response;
        response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
        response << "Content-Type: " << contentType << "\r\n";
        response << "Content-Length: " << body.length() << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        response << body;

        return response.str();
    }

    // Handle client connection
    void handleClient(SOCKET clientSocket) {
        char buffer[8192];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived <= 0) {
            closesocket(clientSocket);
            return;
        }

        buffer[bytesReceived] = '\0';
        std::string requestStr(buffer);
        
        HttpRequest req = parseRequest(requestStr);
        std::string response;

        // Handle CORS preflight
        if (req.method == "OPTIONS") {
            response = createResponse(200, "");
        }
        // Route requests
        else if (req.path == "/api/health") {
            response = handleHealth(req);
        }
        else if (req.path == "/api/junctions") {
            response = handleJunctions(req);
        }
        else if (req.path == "/api/junction" || req.path.rfind("/api/junction/", 0) == 0) {
            response = handleJunction(req);
        }
        else if (req.path == "/api/route") {
            response = handleRoute(req);
        }
        else if (req.path == "/api/roads") {
            response = handleRoads(req);
        }
        else if (req.path == "/api/traffic") {
            response = handleTraffic(req);
        }
        else if (req.path == "/api/stats") {
            response = handleStats(req);
        }
        else if (req.path == "/api/search") {
            response = handleSearch(req);
        }
        else {
            response = createResponse(404, "{\"error\": \"Not Found\"}");
        }

        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }

    // API Handlers
    std::string handleHealth(const HttpRequest& req) {
        std::string json = "{";
        json += "\"status\": \"healthy\",";
        json += "\"service\": \"Smart Traffic Route Optimizer\",";
        json += "\"version\": \"1.0.0\",";
        json += "\"junctions\": " + std::to_string(trafficManager.getJunctionCount()) + ",";
        json += "\"roads\": " + std::to_string(trafficManager.getRoadCount());
        json += "}";
        return createResponse(200, json);
    }

    std::string handleJunctions(const HttpRequest& req) {
        auto junctions = trafficManager.getAllJunctions();
        
        std::string json = "{\"junctions\": [";
        for (size_t i = 0; i < junctions.size(); ++i) {
            json += junctions[i].toJson();
            if (i < junctions.size() - 1) json += ",";
        }
        json += "], \"count\": " + std::to_string(junctions.size()) + "}";
        
        return createResponse(200, json);
    }

    std::string handleJunction(const HttpRequest& req) {
        // Get junction by ID
        if (req.params.find("id") != req.params.end()) {
            int id = std::stoi(req.params.at("id"));
            Junction j;
            if (trafficManager.getJunction(id, &j)) {
                return createResponse(200, j.toJson());
            }
            return createResponse(404, "{\"error\": \"Junction not found\"}");
        }
        
        // Get junction by name
        if (req.params.find("name") != req.params.end()) {
            Junction j;
            if (trafficManager.getJunctionByName(req.params.at("name"), &j)) {
                return createResponse(200, j.toJson());
            }
            return createResponse(404, "{\"error\": \"Junction not found\"}");
        }
        
        return createResponse(400, "{\"error\": \"Missing id or name parameter\"}");
    }

    std::string handleRoute(const HttpRequest& req) {
        if (req.params.find("from") == req.params.end() || 
            req.params.find("to") == req.params.end()) {
            return createResponse(400, "{\"error\": \"Missing from or to parameter\"}");
        }

        int from = std::stoi(req.params.at("from"));
        int to = std::stoi(req.params.at("to"));
        bool useTime = true;
        
        if (req.params.find("optimize") != req.params.end()) {
            useTime = (req.params.at("optimize") == "time");
        }

        RouteResult result = trafficManager.findRoute(from, to, useTime);
        return createResponse(200, result.toJson());
    }

    std::string handleRoads(const HttpRequest& req) {
        auto roads = trafficManager.getAllRoads();
        
        std::string json = "{\"roads\": [";
        for (size_t i = 0; i < roads.size(); ++i) {
            json += roads[i].toJson();
            if (i < roads.size() - 1) json += ",";
        }
        json += "], \"count\": " + std::to_string(roads.size()) + "}";
        
        return createResponse(200, json);
    }

    std::string handleTraffic(const HttpRequest& req) {
        if (req.method == "POST" || req.method == "PUT") {
            // Update traffic level
            if (req.params.find("road") == req.params.end() || 
                req.params.find("level") == req.params.end()) {
                return createResponse(400, "{\"error\": \"Missing road or level parameter\"}");
            }

            int roadId = std::stoi(req.params.at("road"));
            int level = std::stoi(req.params.at("level"));
            
            TrafficLevel trafficLevel;
            switch (level) {
                case 1: trafficLevel = TrafficLevel::LOW; break;
                case 2: trafficLevel = TrafficLevel::NORMAL; break;
                case 3: trafficLevel = TrafficLevel::HEAVY; break;
                case 4: trafficLevel = TrafficLevel::SEVERE; break;
                default: trafficLevel = TrafficLevel::NORMAL;
            }

            if (trafficManager.updateTrafficLevel(roadId, trafficLevel)) {
                return createResponse(200, "{\"success\": true, \"message\": \"Traffic updated\"}");
            }
            return createResponse(404, "{\"error\": \"Road not found\"}");
        }

        // GET - return all traffic levels
        auto roads = trafficManager.getAllRoads();
        std::string json = "{\"traffic\": [";
        for (size_t i = 0; i < roads.size(); ++i) {
            json += "{";
            json += "\"roadId\": " + std::to_string(roads[i].id) + ",";
            json += "\"name\": \"" + roads[i].name + "\",";
            json += "\"level\": \"" + trafficLevelToString(roads[i].trafficLevel) + "\",";
            json += "\"multiplier\": " + std::to_string(getTrafficMultiplier(roads[i].trafficLevel));
            json += "}";
            if (i < roads.size() - 1) json += ",";
        }
        json += "]}";
        return createResponse(200, json);
    }

    std::string handleStats(const HttpRequest& req) {
        std::string json = "{";
        json += "\"junctions\": " + std::to_string(trafficManager.getJunctionCount()) + ",";
        json += "\"roads\": " + std::to_string(trafficManager.getRoadCount()) + ",";
        json += "\"cacheHitRate\": " + std::to_string(trafficManager.getCacheHitRate());
        json += "}";
        return createResponse(200, json);
    }

    std::string handleSearch(const HttpRequest& req) {
        if (req.params.find("q") == req.params.end()) {
            return createResponse(400, "{\"error\": \"Missing q parameter\"}");
        }

        auto results = trafficManager.searchJunctions(req.params.at("q"));
        
        std::string json = "{\"results\": [";
        for (size_t i = 0; i < results.size(); ++i) {
            json += results[i].toJson();
            if (i < results.size() - 1) json += ",";
        }
        json += "], \"count\": " + std::to_string(results.size()) + "}";
        
        return createResponse(200, json);
    }

public:
    HttpServer(int p, TrafficManager& tm) : port(p), trafficManager(tm), running(false) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }

    ~HttpServer() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool start() {
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket\n";
            return false;
        }

        // Allow socket reuse
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Error binding socket\n";
            closesocket(serverSocket);
            return false;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Error listening on socket\n";
            closesocket(serverSocket);
            return false;
        }

        running = true;
        std::cout << "\n========================================\n";
        std::cout << " Smart Traffic Route Optimizer API\n";
        std::cout << " Server running on http://localhost:" << port << "\n";
        std::cout << "========================================\n";
        std::cout << "\nEndpoints:\n";
        std::cout << "  GET  /api/health     - Health check\n";
        std::cout << "  GET  /api/junctions  - Get all junctions\n";
        std::cout << "  GET  /api/junction   - Get junction by id or name\n";
        std::cout << "  GET  /api/roads      - Get all roads\n";
        std::cout << "  GET  /api/route      - Find route (from, to, optimize)\n";
        std::cout << "  GET  /api/traffic    - Get traffic levels\n";
        std::cout << "  POST /api/traffic    - Update traffic level\n";
        std::cout << "  GET  /api/search     - Search junctions\n";
        std::cout << "  GET  /api/stats      - System statistics\n";
        std::cout << "\nPress Ctrl+C to stop the server.\n\n";

        return true;
    }

    void run() {
        while (running) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    std::cerr << "Error accepting connection\n";
                }
                continue;
            }

            // Handle client in a new thread
            std::thread([this, clientSocket]() {
                handleClient(clientSocket);
            }).detach();
        }
    }

    void stop() {
        running = false;
        closesocket(serverSocket);
    }

    bool isRunning() const { return running; }
};

#endif // HTTPSERVER_H
