#include "RateLimitFilter.h"

using namespace drogon;

void RateLimitFilter::cleanupOldEntries()
{
    auto now = std::chrono::steady_clock::now();
    
    // Only cleanup every 60 seconds
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastCleanup_).count() < CLEANUP_INTERVAL_SECONDS) {
        return;
    }
    
    lastCleanup_ = now;
    
    // Remove expired entries
    for (auto it = rateLimits_.begin(); it != rateLimits_.end();) {
        if (now > it->second.resetTime) {
            it = rateLimits_.erase(it);
        } else {
            ++it;
        }
    }
}

void RateLimitFilter::doFilter(const HttpRequestPtr &req,
                               FilterCallback &&fcb,
                               FilterChainCallback &&fccb)
{
    // Get client IP
    std::string clientIp = req->getPeerAddr().toIp();
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Cleanup old entries periodically
    cleanupOldEntries();
    
    auto now = std::chrono::steady_clock::now();
    
    // Check if IP exists in rate limit map
    auto it = rateLimits_.find(clientIp);
    
    if (it == rateLimits_.end()) {
        // First request from this IP
        rateLimits_[clientIp] = {
            1,
            now + std::chrono::minutes(1)
        };
        fccb();
        return;
    }
    
    // Check if rate limit window has expired
    if (now > it->second.resetTime) {
        // Reset the counter
        it->second.count = 1;
        it->second.resetTime = now + std::chrono::minutes(1);
        fccb();
        return;
    }
    
    // Check if limit exceeded
    if (it->second.count >= MAX_REQUESTS_PER_MINUTE) {
        LOG_WARN << "Rate limit exceeded for IP: " << clientIp;
        
        Json::Value error;
        error["error"] = "Too many requests";
        error["message"] = "Rate limit exceeded. Maximum " + std::to_string(MAX_REQUESTS_PER_MINUTE) + " requests per minute";
        
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k429TooManyRequests);
        resp->addHeader("Retry-After", "60");
        fcb(resp);
        return;
    }
    
    // Increment counter
    it->second.count++;
    
    // Continue with request
    fccb();
}
