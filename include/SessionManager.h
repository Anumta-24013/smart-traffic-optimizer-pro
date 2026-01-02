/**
 * Smart Traffic Route Optimizer
 * Session Manager - Token Validation & Management
 * 
 * Handles user sessions, token validation, and expiry
 */

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <random>
#include "Models.h"

class SessionManager {
private:
    struct Session {
        std::string token;
        int userId;
        std::string username;
        long long createdAt;
        long long expiresAt;
        std::string ipAddress;
        
        bool isExpired() const {
            auto now = std::chrono::system_clock::now();
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count();
            return nowMs > expiresAt;
        }
    };

    std::unordered_map<std::string, Session> sessions;  // token -> Session
    std::unordered_map<int, std::string> userSessions;  // userId -> token
    mutable std::mutex sessionMutex;

    // Token expiry duration (24 hours)
    static const long long TOKEN_EXPIRY_MS = 24 * 60 * 60 * 1000;

    // Generate random token
    std::string generateToken() {
        static const char alphanum[] =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
        
        std::string token;
        for (int i = 0; i < 32; ++i) {
            token += alphanum[dis(gen)];
        }
        return token;
    }

    long long getCurrentTimeMs() const {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ).count();
    }

public:
    SessionManager() = default;

    // Create new session for user
    std::string createSession(int userId, const std::string& username, 
                              const std::string& ipAddress = "") {
        std::lock_guard<std::mutex> lock(sessionMutex);
        
        // Check if user already has active session
        auto it = userSessions.find(userId);
        if (it != userSessions.end()) {
            // Invalidate old session
            sessions.erase(it->second);
        }
        
        // Create new session
        std::string token = generateToken();
        long long now = getCurrentTimeMs();
        
        Session session;
        session.token = token;
        session.userId = userId;
        session.username = username;
        session.createdAt = now;
        session.expiresAt = now + TOKEN_EXPIRY_MS;
        session.ipAddress = ipAddress;
        
        sessions[token] = session;
        userSessions[userId] = token;
        
        std::cout << "✅ Session created for user: " << username 
                  << " (expires in 24h)\n";
        
        return token;
    }

    // Validate token and get user info
    bool validateToken(const std::string& token, int* userId = nullptr, 
                       std::string* username = nullptr) const {
        std::lock_guard<std::mutex> lock(sessionMutex);
        
        auto it = sessions.find(token);
        if (it == sessions.end()) {
            return false;  // Token not found
        }
        
        if (it->second.isExpired()) {
            return false;  // Token expired
        }
        
        if (userId) *userId = it->second.userId;
        if (username) *username = it->second.username;
        
        return true;
    }

    // Get user ID from token
    int getUserId(const std::string& token) const {
        int userId = -1;
        validateToken(token, &userId);
        return userId;
    }

    // Invalidate session (logout)
    bool invalidateSession(const std::string& token) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        
        auto it = sessions.find(token);
        if (it == sessions.end()) {
            return false;
        }
        
        int userId = it->second.userId;
        std::string username = it->second.username;
        
        sessions.erase(it);
        userSessions.erase(userId);
        
        std::cout << "🔒 Session invalidated for user: " << username << "\n";
        
        return true;
    }

    // Invalidate all sessions for a user
    bool invalidateUserSessions(int userId) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        
        auto it = userSessions.find(userId);
        if (it == userSessions.end()) {
            return false;
        }
        
        sessions.erase(it->second);
        userSessions.erase(it);
        
        return true;
    }

    // Clean expired sessions (call periodically)
    int cleanExpiredSessions() {
        std::lock_guard<std::mutex> lock(sessionMutex);
        
        int cleaned = 0;
        auto it = sessions.begin();
        
        while (it != sessions.end()) {
            if (it->second.isExpired()) {
                int userId = it->second.userId;
                userSessions.erase(userId);
                it = sessions.erase(it);
                cleaned++;
            } else {
                ++it;
            }
        }
        
        if (cleaned > 0) {
            std::cout << "🧹 Cleaned " << cleaned << " expired sessions\n";
        }
        
        return cleaned;
    }

    // Get active session count
    size_t getActiveSessionCount() const {
        std::lock_guard<std::mutex> lock(sessionMutex);
        return sessions.size();
    }

    // Get session info
    bool getSessionInfo(const std::string& token, Session* info) const {
        std::lock_guard<std::mutex> lock(sessionMutex);
        
        auto it = sessions.find(token);
        if (it == sessions.end() || it->second.isExpired()) {
            return false;
        }
        
        if (info) *info = it->second;
        return true;
    }

    // Check if user is logged in
    bool isUserLoggedIn(int userId) const {
        std::lock_guard<std::mutex> lock(sessionMutex);
        return userSessions.find(userId) != userSessions.end();
    }

    // Get all active users
    std::vector<std::string> getActiveUsers() const {
        std::lock_guard<std::mutex> lock(sessionMutex);
        
        std::vector<std::string> users;
        for (const auto& pair : sessions) {
            if (!pair.second.isExpired()) {
                users.push_back(pair.second.username);
            }
        }
        return users;
    }

    // Extract token from Authorization header
    static std::string extractTokenFromHeader(const std::string& authHeader) {
        // Expected format: "Bearer <token>" or just "<token>"
        if (authHeader.empty()) {
            return "";
        }
        
        if (authHeader.find("Bearer ") == 0) {
            return authHeader.substr(7);  // Remove "Bearer "
        }
        
        return authHeader;
    }

    // Print statistics
    void printStats() const {
        std::lock_guard<std::mutex> lock(sessionMutex);
        
        int expired = 0;
        for (const auto& pair : sessions) {
            if (pair.second.isExpired()) {
                expired++;
            }
        }
        
        std::cout << "📊 Session Statistics:\n";
        std::cout << "   Active: " << (sessions.size() - expired) << "\n";
        std::cout << "   Expired: " << expired << "\n";
        std::cout << "   Total: " << sessions.size() << "\n";
    }
};

#endif // SESSIONMANAGER_H