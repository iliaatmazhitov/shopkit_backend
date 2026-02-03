#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class OrderController : public HttpController<OrderController>
{
public:
    METHOD_LIST_BEGIN

        // Public API
        ADD_METHOD_TO(OrderController::createOrder,
                      "/api/orders",
                      Post);

        ADD_METHOD_TO(OrderController::getOrders,
                      "/api/orders/{1}",
                      Get);

        ADD_METHOD_TO(OrderController::getOrderDetail,
                      "/api/orders/detail/{1}",
                      Get);

        // Admin API
        ADD_METHOD_TO(OrderController::updateOrderStatus,
                      "/api/admin/orders/{1}/status",
                      Put);

    METHOD_LIST_END

    void createOrder(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback);

    void getOrders(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   int shop_id);

    void getOrderDetail(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback,
                        const std::string &order_id);

    void updateOrderStatus(const HttpRequestPtr &req,
                          std::function<void(const HttpResponsePtr &)> &&callback,
                          const std::string &order_id);
};
