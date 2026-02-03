#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class ProductController : public HttpController<ProductController>
{
public:
    METHOD_LIST_BEGIN

        // Public API
        ADD_METHOD_TO(ProductController::getProducts,
                      "/api/products/{1}",
                      Get);

        ADD_METHOD_TO(ProductController::getProductDetail,
                      "/api/products/detail/{1}",
                      Get);

        // Admin API
        ADD_METHOD_TO(ProductController::createProduct,
                      "/api/admin/products",
                      Post);

        ADD_METHOD_TO(ProductController::updateProduct,
                      "/api/admin/products/{1}",
                      Put);

        ADD_METHOD_TO(ProductController::deleteProduct,
                      "/api/admin/products/{1}",
                      Delete);

    METHOD_LIST_END

    void getProducts(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback,
                     int shop_id);

    void getProductDetail(const HttpRequestPtr &req,
                          std::function<void(const HttpResponsePtr &)> &&callback,
                          const std::string &product_id);

    void createProduct(const HttpRequestPtr &req,
                       std::function<void(const HttpResponsePtr &)> &&callback);

    void updateProduct(const HttpRequestPtr &req,
                       std::function<void(const HttpResponsePtr &)> &&callback,
                       const std::string &product_id);

    void deleteProduct(const HttpRequestPtr &req,
                       std::function<void(const HttpResponsePtr &)> &&callback,
                       const std::string &product_id);
};