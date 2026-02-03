#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class OrderController : public HttpController<OrderController>
{
public:
    METHOD_LIST_BEGIN

        ADD_METHOD_TO(OrderController::createOrder,
                      "/api/orders",
                      Post);

        ADD_METHOD_TO(OrderController::getUserOrders,
                      "/api/user/orders/{1}",
                      Get);

    METHOD_LIST_END

    void createOrder(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback);

    void getUserOrders(const HttpRequestPtr &req,
                       std::function<void(const HttpResponsePtr &)> &&callback,
                       int64_t telegram_user_id);
};
