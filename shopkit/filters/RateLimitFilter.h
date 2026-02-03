#pragma once
#include <drogon/HttpFilter.h>
#include <unordered_map>
#include <mutex>
#include <chrono>

using namespace drogon;

class RateLimitFilter : public HttpFilter<RateLimitFilter>
{
public:
    RateLimitFilter() : lastCleanup_(std::chrono::steady_clock::now()) {}
    
    void doFilter(const HttpRequestPtr &req,
                 FilterCallback &&fcb,
                 FilterChainCallback &&fccb) override;

private:
    struct RateLimitInfo {
        int count;
        std::chrono::steady_clock::time_point resetTime;
    };

    std::unordered_map<std::string, RateLimitInfo> rateLimits_;
    std::mutex mutex_;
    std::chrono::steady_clock::time_point lastCleanup_;
    
    const int MAX_REQUESTS_PER_MINUTE = 100;
    const int CLEANUP_INTERVAL_SECONDS = 60;
    
    void cleanupOldEntries();
};
