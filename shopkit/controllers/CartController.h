#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class CartController : public HttpController<CartController>
{
public:
    METHOD_LIST_BEGIN

        ADD_METHOD_TO(CartController::saveCart,
                      "/api/cart",
                      Post);

        ADD_METHOD_TO(CartController::getCart,
                      "/api/cart/{1}",
                      Get);

        ADD_METHOD_TO(CartController::clearCart,
                      "/api/cart/{1}",
                      Delete);

    METHOD_LIST_END

    void saveCart(const HttpRequestPtr &req,
                  std::function<void(const HttpResponsePtr &)> &&callback);

    void getCart(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback,
                 int64_t telegram_user_id);

    void clearCart(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   int64_t telegram_user_id);
};
