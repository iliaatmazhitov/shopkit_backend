#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class ShopController : public HttpController<ShopController>
{
public:
    METHOD_LIST_BEGIN

        ADD_METHOD_TO(ShopController::getShopByToken,
                      "/api/shops/token/{1}",
                      Get);

        ADD_METHOD_TO(ShopController::createShop,
                      "/api/shops",
                      Post);

        ADD_METHOD_TO(ShopController::updateShop,
                      "/api/shops/{1}",
                      Put);
    METHOD_LIST_END

    void getShopByToken(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback,
                        const std::string &shop_token);

    void createShop(const HttpRequestPtr &req,
                    std::function<void(const HttpResponsePtr &)> &&callback);

    void updateShop(const HttpRequestPtr &req,
                    std::function<void(const HttpResponsePtr &)> &&callback,
                    const std::string &shop_token);
};