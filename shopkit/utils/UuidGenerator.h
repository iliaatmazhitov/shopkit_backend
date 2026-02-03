#pragma once
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

class UuidGenerator {
public:
    static std::string generate() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        uint64_t random1 = dis(gen);
        uint64_t random2 = dis(gen);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        ss << std::setw(8) << ((random1 >> 32) & 0xFFFFFFFF) << "-";
        ss << std::setw(4) << ((random1 >> 16) & 0xFFFF) << "-";
        ss << "4" << std::setw(3) << ((random1 >> 4) & 0xFFF) << "-";
        ss << std::setw(1) << (8 | (random2 >> 62 & 0x3))
           << std::setw(3) << ((random2 >> 48) & 0xFFF) << "-";
        ss << std::setw(12) << (random2 & 0xFFFFFFFFFFFF);

        return ss.str();
    }
};
