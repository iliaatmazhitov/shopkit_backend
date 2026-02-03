#include <drogon/drogon.h>
#include <iostream>
#include <cstdlib>
#include <regex>

int main() {
    std::cout << "========= ShopKit Backend Starting =========" << std::endl;
    
    try {
        // Порт сервера
        int port = 8080;
        if (std::getenv("PORT")) {
            port = std::stoi(std::getenv("PORT"));
        }
        
        drogon::app()
            .setLogLevel(trantor::Logger::kInfo)
            .addListener("0.0.0.0", port)
            .setThreadNum(4);
        
        // Парсинг DATABASE_URL от Railway
        std::string database_url = std::getenv("DATABASE_URL") ? std::getenv("DATABASE_URL") : "";
        
        if (database_url.empty()) {
            std::cerr << "❌ ERROR: DATABASE_URL not set!" << std::endl;
            std::cerr << "Set it in Railway dashboard or use local config" << std::endl;
            return 1;
        }
        
        std::cout << "DATABASE_URL found, parsing..." << std::endl;
        
        // Regex для парсинга: postgresql://user:password@host:port/database
        std::regex url_regex("postgres(?:ql)?://([^:]+):([^@]*)@([^:]+):(\\d+)/([^?]+)");
        std::smatch matches;
        
        if (!std::regex_search(database_url, matches, url_regex)) {
            std::cerr << "❌ ERROR: Invalid DATABASE_URL format!" << std::endl;
            std::cerr << "Expected: postgresql://user:password@host:port/database" << std::endl;
            return 1;
        }
        
        std::string db_user = matches[1];
        std::string db_password = matches[2];
        std::string db_host = matches[3];
        int db_port = std::stoi(matches[4]);
        std::string db_name = matches[5];
        
        std::cout << "Database config:" << std::endl;
        std::cout << "  Host: " << db_host << std::endl;
        std::cout << "  Port: " << db_port << std::endl;
        std::cout << "  Database: " << db_name << std::endl;
        std::cout << "  User: " << db_user << std::endl;
        
        // Создаём клиент БД
        drogon::app().createDbClient(
            "postgresql",
            db_host,
            db_port,
            db_name,
            db_user,
            db_password,
            10,           // connection pool size
            "",           // filename (unused for PostgreSQL)
            "default",    // client name
            false,        // is_fast mode
            "utf8"        // character set
        );
        
        std::cout << "✅ Database client created successfully" << std::endl;
        std::cout << "🚀 Starting HTTP server on http://0.0.0.0:" << port << std::endl;
        
        drogon::app().run();
        
    } catch (const std::exception &e) {
        std::cerr << "❌ Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
