#include <drogon/drogon.h>
#include <iostream>
#include <cstdlib>
#include <regex>

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "ShopKit Backend Starting" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        int port = 8080;
        if (std::getenv("PORT")) {
            port = std::stoi(std::getenv("PORT"));
        }
        std::cout << "[INFO] Server port: " << port << std::endl;
        
        std::cout << "\n[DEBUG] Checking environment variables:" << std::endl;
        const char* db_url = std::getenv("DATABASE_URL");
        std::cout << "DATABASE_URL: " << (db_url ? "SET" : "NOT SET") << std::endl;
        if (db_url) {
            std::string url_str(db_url);
            size_t at_pos = url_str.find("@");
            if (at_pos != std::string::npos) {
                std::cout << "DATABASE_URL format: " << url_str.substr(0, url_str.find("://") + 3) 
                         << "****:****" << url_str.substr(at_pos) << std::endl;
            }
        }
        
        const char* pghost = std::getenv("PGHOST");
        const char* pgport = std::getenv("PGPORT");
        const char* pgdb = std::getenv("PGDATABASE");
        const char* pguser = std::getenv("PGUSER");
        const char* pgpass = std::getenv("PGPASSWORD");
        
        std::cout << "PGHOST: " << (pghost ? pghost : "NOT SET") << std::endl;
        std::cout << "PGPORT: " << (pgport ? pgport : "NOT SET") << std::endl;
        std::cout << "PGDATABASE: " << (pgdb ? pgdb : "NOT SET") << std::endl;
        std::cout << "PGUSER: " << (pguser ? pguser : "NOT SET") << std::endl;
        std::cout << "PGPASSWORD: " << (pgpass ? "SET" : "NOT SET") << std::endl;
        
        drogon::app()
            .setLogLevel(trantor::Logger::kInfo)
            .addListener("0.0.0.0", port)
            .setThreadNum(4);
        
        std::string db_host, db_name, db_user, db_password;
        int db_port = 5432;
        
        if (pghost && pgdb && pguser && pgpass) {
            std::cout << "\n[INFO] Using individual PG* variables" << std::endl;
            db_host = pghost;
            db_name = pgdb;
            db_user = pguser;
            db_password = pgpass;
            if (pgport) {
                db_port = std::stoi(pgport);
            }
        }
        else if (db_url) {
            std::cout << "\n[INFO] Parsing DATABASE_URL" << std::endl;
            std::string database_url(db_url);
            
            // ✅ ОБНОВЛЁННЫЙ REGEX: порт опционален
            std::regex url_regex("postgres(?:ql)?://([^:]+):([^@]*)@([^:/]+)(?::(\\d+))?/([^?]+)");
            std::smatch matches;
            
            if (std::regex_search(database_url, matches, url_regex)) {
                db_user = matches[1];
                db_password = matches[2];
                db_host = matches[3];
                
                // Порт опционален (группа 4)
                if (matches[4].matched && !matches[4].str().empty()) {
                    db_port = std::stoi(matches[4]);
                } else {
                    db_port = 5432; // По умолчанию
                }
                
                db_name = matches[5];
                std::cout << "[INFO] Parsed successfully" << std::endl;
            } else {
                std::cerr << "[ERROR] Failed to parse DATABASE_URL!" << std::endl;
                std::cerr << "[ERROR] Expected format: postgresql://user:password@host[:port]/database" << std::endl;
                return 1;
            }
        } else {
            std::cerr << "[ERROR] No database configuration found!" << std::endl;
            std::cerr << "[ERROR] Set DATABASE_URL or PG* variables" << std::endl;
            return 1;
        }
        
        std::cout << "\n[INFO] Database configuration:" << std::endl;
        std::cout << "  Host: " << db_host << std::endl;
        std::cout << "  Port: " << db_port << std::endl;
        std::cout << "  Database: " << db_name << std::endl;
        std::cout << "  User: " << db_user << std::endl;
        std::cout << "  Password: " << (db_password.empty() ? "EMPTY" : "SET") << std::endl;
        
        std::cout << "\n[INFO] Creating database client..." << std::endl;
        
        try {
            drogon::app().createDbClient(
                "postgresql",
                db_host,
                db_port,
                db_name,
                db_user,
                db_password,
                10,
                "",
                "default",
                false,
                "utf8"
            );
            std::cout << "[SUCCESS] Database client created!" << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "[ERROR] Failed to create DB client: " << e.what() << std::endl;
            return 1;
        }
        
        std::cout << "\n[SUCCESS] Starting HTTP server on http://0.0.0.0:" << port << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        drogon::app().run();
        
    } catch (const std::exception &e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
