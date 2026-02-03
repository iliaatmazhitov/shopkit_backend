#pragma once
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

class UuidGenerator {
public:
    static std::string generate() {
        thread_local std::random_device rd;
        thread_local std::mt19937_64 gen(rd());
        thread_local std::uniform_int_distribution<uint64_t> dis;

        // Генерируем UUID v4 формат: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
        uint64_t random1 = dis(gen);
        uint64_t random2 = dis(gen);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        // Первые 8 символов
        ss << std::setw(8) << ((random1 >> 32) & 0xFFFFFFFF) << "-";
        
        // Следующие 4
        ss << std::setw(4) << ((random1 >> 16) & 0xFFFF) << "-";
        
        // 4 символа с версией 4
        ss << "4" << std::setw(3) << ((random1 >> 4) & 0xFFF) << "-";
        
        // 4 символа с вариантом
        ss << std::setw(1) << (8 | ((random2 >> 62) & 0x3))
           << std::setw(3) << ((random2 >> 48) & 0xFFF) << "-";
        
        // Последние 12 символов
        ss << std::setw(12) << (random2 & 0xFFFFFFFFFFFF);

        return ss.str();
    }
};
