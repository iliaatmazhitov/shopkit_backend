#include "CorsFilter.h"

using namespace drogon;

void CorsFilter::doFilter(const HttpRequestPtr &req,
                         FilterCallback &&fcb,
                         FilterChainCallback &&fccb)
{
    // Handle OPTIONS (preflight) requests
    if (req->method() == Options) {
        auto resp = HttpResponse::newHttpResponse();
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, X-Owner-TG-ID");
        resp->addHeader("Access-Control-Max-Age", "86400");
        resp->setStatusCode(k204NoContent);
        fcb(resp);
        return;
    }

    // Continue with the request and add CORS headers to response
    fccb();
}
