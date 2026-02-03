#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class OrderController : public HttpController<OrderController>
{
public:
    METHOD_LIST_BEGIN
    
    // Public API (for customers)
    ADD_METHOD_TO(OrderController::createOrder, 
                  "/api/orders", 
                  Post);
    
    ADD_METHOD_TO(OrderController::getOrder, 
                  "/api/orders/{1}", 
                  Get);
    
    // Admin API (for shop owners)
    ADD_METHOD_TO(OrderController::getShopOrders, 
                  "/api/admin/orders/{1}", 
                  Get);
    
    ADD_METHOD_TO(OrderController::updateOrderStatus, 
                  "/api/admin/orders/{1}", 
                  Put);
    
    METHOD_LIST_END

    void createOrder(const HttpRequestPtr &req,
                    std::function<void(const HttpResponsePtr &)> &&callback);
    
    void getOrder(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback,
                 const std::string &order_id);
    
    void getShopOrders(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback,
                      int shop_id);
    
    void updateOrderStatus(const HttpRequestPtr &req,
                          std::function<void(const HttpResponsePtr &)> &&callback,
                          const std::string &order_id);
};
