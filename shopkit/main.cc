#include <drogon/drogon.h>
#include <iostream>

int main() {
    std::cout << "========= ShopKit Server Starting =========" << std::endl;
    
    try {
        drogon::app()
            .setLogLevel(trantor::Logger::kDebug)
            .loadConfigFile("config.json");
        
        auto dbClient = drogon::app().getDbClient();
        if (!dbClient) {
            std::cerr << "FATAL: Database client is NULL!" << std::endl;
            return 1;
        }
        
        std::cout << "Database: OK" << std::endl;
        std::cout << "Starting server on 0.0.0.0:8080" << std::endl;
        
        drogon::app().run();
        
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
