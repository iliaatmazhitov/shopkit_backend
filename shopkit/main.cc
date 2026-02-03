#include <drogon/drogon.h>
#include <iostream>

int main() {
    std::cout << "========= ShopKit Backend Starting =========" << std::endl;
    
    try {
        drogon::app()
            .setLogLevel(trantor::Logger::kDebug)
            .addListener("0.0.0.0", 8080)
            .setThreadNum(4);
        
        // Create database client directly (config.json db settings will be used automatically if available)
        drogon::app().createDbClient(
            "postgresql",       // rdbms type
            "127.0.0.1",        // host
            5432,               // port
            "shopkit",          // database name
            "ilya",             // user
            "",                 // password (empty)
            10,                 // connection pool size
            "",                 // filename (unused for PostgreSQL)
            "default",          // client name
            false,              // is_fast mode
            "utf8"              // character set
        );
        
        std::cout << "✅ Database configured: postgresql://ilya@127.0.0.1:5432/shopkit" << std::endl;
        std::cout << "🚀 Starting HTTP server on http://0.0.0.0:8080" << std::endl;
        
        // Now run the app - DB client will be available inside controllers
        drogon::app().run();
        
    } catch (const std::exception &e) {
        std::cerr << "❌ Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Server stopped" << std::endl;
    return 0;
}
