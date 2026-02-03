#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class OrderController : public HttpController<OrderController>
{
public:
    METHOD_LIST_BEGIN

        // Public API - Create order
        ADD_METHOD_TO(OrderController::createOrder,
                      "/api/orders",
                      Post);

        // Admin API - Get orders for shop
        ADD_METHOD_TO(OrderController::getOrders,
                      "/api/admin/orders/{1}",
                      Get);

        // Admin API - Update order status
        ADD_METHOD_TO(OrderController::updateOrderStatus,
                      "/api/admin/orders/{1}",
                      Put);

    METHOD_LIST_END

    void createOrder(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback);

    void getOrders(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   int shop_id);

    void updateOrderStatus(const HttpRequestPtr &req,
                          std::function<void(const HttpResponsePtr &)> &&callback,
                          const std::string &order_id);
};
