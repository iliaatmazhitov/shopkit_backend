#include "ProductController.h"
#include <drogon/orm/DbClient.h>

using namespace drogon;
using namespace drogon::orm;

void ProductController::getProducts(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        int shop_id)
{
    LOG_INFO << "GET /api/products/" << shop_id;

    auto dbClient = app().getDbClient();
    if (!dbClient) {
        LOG_ERROR << "Database client is null!";
        Json::Value error;
        error["error"] = "Database not configured";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }

    std::string sql = R"(
        SELECT product_id, title, price, currency, image_url, description, stock_count, category
        FROM products
        WHERE shop_id = $1 AND is_active = true
        ORDER BY created_at DESC
    )";

    dbClient->execSqlAsync(
            sql,
            [callback](const Result &result) {
                Json::Value response(Json::arrayValue);

                for (const auto &row : result) {
                    Json::Value product;
                    product["product_id"] = row["product_id"].as<std::string>();
                    product["title"] = row["title"].as<std::string>();
                    product["price"] = row["price"].as<int>();
                    product["currency"] = row["currency"].as<std::string>();
                    product["image_url"] = row["image_url"].isNull() ? "" : row["image_url"].as<std::string>();
                    product["description"] = row["description"].isNull() ? "" : row["description"].as<std::string>();
                    product["stock_count"] = row["stock_count"].isNull() ? 0 : row["stock_count"].as<int>();
                    product["category"] = row["category"].isNull() ? "" : row["category"].as<std::string>();
                    response.append(product);
                }

                auto resp = HttpResponse::newHttpJsonResponse(response);
                resp->addHeader("Access-Control-Allow-Origin", "*");
                callback(resp);
            },
            [callback](const DrogonDbException &e) {
                LOG_ERROR << "DB error: " << e.base().what();
                Json::Value error;
                error["error"] = "Database error";
                error["code"] = "DB_ERROR";
                error["message"] = e.base().what();
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
            },
            shop_id
    );
}

void ProductController::getProductDetail(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        const std::string &product_id)
{
    LOG_INFO << "GET /api/products/detail/" << product_id;

    auto dbClient = app().getDbClient();
    if (!dbClient) {
        Json::Value error;
        error["error"] = "Database not configured";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }

    std::string sql = R"(
        SELECT p.*, s.title as shop_title, s.shop_token
        FROM products p
        LEFT JOIN shops s ON p.shop_id = s.id
        WHERE p.product_id = $1 AND p.is_active = true
    )";

    dbClient->execSqlAsync(
            sql,
            [callback](const Result &result) {
                if (result.empty()) {
                    Json::Value error;
                    error["error"] = "Product not found";
                    error["code"] = "PRODUCT_NOT_FOUND";
                    auto resp = HttpResponse::newHttpJsonResponse(error);
                    resp->setStatusCode(k404NotFound);
                    callback(resp);
                    return;
                }

                const auto &row = result[0];
                Json::Value product;
                product["product_id"] = row["product_id"].as<std::string>();
                product["title"] = row["title"].as<std::string>();
                product["price"] = row["price"].as<int>();
                product["currency"] = row["currency"].as<std::string>();
                product["image_url"] = row["image_url"].isNull() ? "" : row["image_url"].as<std::string>();
                product["description"] = row["description"].isNull() ? "" : row["description"].as<std::string>();
                product["stock_count"] = row["stock_count"].isNull() ? 0 : row["stock_count"].as<int>();
                product["category"] = row["category"].isNull() ? "" : row["category"].as<std::string>();

                product["shop"] = Json::objectValue;
                product["shop"]["id"] = row["shop_id"].as<int>();
                product["shop"]["title"] = row["shop_title"].as<std::string>();
                product["shop"]["shop_token"] = row["shop_token"].as<std::string>();

                auto resp = HttpResponse::newHttpJsonResponse(product);
                resp->addHeader("Access-Control-Allow-Origin", "*");
                callback(resp);
            },
            [callback](const DrogonDbException &e) {
                LOG_ERROR << "DB error: " << e.base().what();
                Json::Value error;
                error["error"] = "Database error";
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
            },
            product_id
    );
}

void ProductController::createProduct(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_INFO << "POST /api/admin/products";

    auto json = req->getJsonObject();
    if (!json || !json->isMember("shop_id") || !json->isMember("title") || !json->isMember("price")) {
        Json::Value error;
        error["error"] = "Missing required fields: shop_id, title, price";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    int shop_id = (*json)["shop_id"].asInt();
    std::string title = (*json)["title"].asString();
    int price = (*json)["price"].asInt();
    std::string image_url = json->get("image_url", "").asString();
    std::string description = json->get("description", "").asString();
    std::string currency = json->get("currency", "RUB").asString();
    int stock_count = json->get("stock_count", 0).asInt();
    std::string category = json->get("category", "").asString();

    auto dbClient = app().getDbClient();
    if (!dbClient) {
        Json::Value error;
        error["error"] = "Database not configured";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }

    std::string sql = R"(
        INSERT INTO products (shop_id, title, price, currency, image_url, description, stock_count, category)
        VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
        RETURNING product_id, created_at
    )";

    dbClient->execSqlAsync(
            sql,
            [callback, shop_id, title, price, currency](const Result &result) {
                Json::Value response;
                response["success"] = true;
                response["product"] = Json::objectValue;
                response["product"]["product_id"] = result[0]["product_id"].as<std::string>();
                response["product"]["shop_id"] = shop_id;
                response["product"]["title"] = title;
                response["product"]["price"] = price;
                response["product"]["currency"] = currency;
                response["product"]["created_at"] = result[0]["created_at"].as<std::string>();

                auto resp = HttpResponse::newHttpJsonResponse(response);
                resp->setStatusCode(k201Created);
                callback(resp);
            },
            [callback](const DrogonDbException &e) {
                LOG_ERROR << "DB error: " << e.base().what();
                Json::Value error;
                error["error"] = "Failed to create product";
                error["code"] = "DB_ERROR";
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
            },
            shop_id, title, price, currency, image_url, description, stock_count, category
    );
}

void ProductController::updateProduct(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        const std::string &product_id)
{
    LOG_INFO << "PUT /api/admin/products/" << product_id;

    auto json = req->getJsonObject();
    if (!json || json->empty()) {
        Json::Value error;
        error["error"] = "No fields to update";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    auto dbClient = app().getDbClient();
    if (!dbClient) {
        Json::Value error;
        error["error"] = "Database not configured";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }

    std::vector<std::string> setClauses;
    int paramIndex = 1;

    std::string title_val, description_val, image_url_val, category_val;
    int price_val = 0, stock_count_val = 0;
    bool has_title = false, has_price = false, has_description = false;
    bool has_image_url = false, has_stock_count = false, has_category = false;

    if (json->isMember("title")) {
        setClauses.push_back("title = $" + std::to_string(paramIndex++));
        title_val = (*json)["title"].asString();
        has_title = true;
    }

    if (json->isMember("price")) {
        setClauses.push_back("price = $" + std::to_string(paramIndex++));
        price_val = (*json)["price"].asInt();
        has_price = true;
    }

    if (json->isMember("description")) {
        setClauses.push_back("description = $" + std::to_string(paramIndex++));
        description_val = (*json)["description"].asString();
        has_description = true;
    }

    if (json->isMember("image_url")) {
        setClauses.push_back("image_url = $" + std::to_string(paramIndex++));
        image_url_val = (*json)["image_url"].asString();
        has_image_url = true;
    }

    if (json->isMember("stock_count")) {
        setClauses.push_back("stock_count = $" + std::to_string(paramIndex++));
        stock_count_val = (*json)["stock_count"].asInt();
        has_stock_count = true;
    }

    if (json->isMember("category")) {
        setClauses.push_back("category = $" + std::to_string(paramIndex++));
        category_val = (*json)["category"].asString();
        has_category = true;
    }

    if (setClauses.empty()) {
        Json::Value error;
        error["error"] = "No valid fields to update";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    setClauses.push_back("updated_at = CURRENT_TIMESTAMP");

    std::string setClause;
    for (size_t i = 0; i < setClauses.size(); ++i) {
        if (i > 0) setClause += ", ";
        setClause += setClauses[i];
    }

    std::string sql = "UPDATE products SET " + setClause +
                      " WHERE product_id = $" + std::to_string(paramIndex) +
                      " RETURNING title, price, updated_at";

    auto executeUpdate = [=, callback = std::move(callback)]() mutable {
        auto binder = *dbClient << sql;

        if (has_title) binder << title_val;
        if (has_price) binder << price_val;
        if (has_description) binder << description_val;
        if (has_image_url) binder << image_url_val;
        if (has_stock_count) binder << stock_count_val;
        if (has_category) binder << category_val;

        binder << product_id;

        binder >> [callback, product_id](const Result &result) {
            if (result.empty()) {
                Json::Value error;
                error["error"] = "Product not found";
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k404NotFound);
                callback(resp);
                return;
            }

            Json::Value response;
            response["success"] = true;
            response["product_id"] = product_id;
            response["title"] = result[0]["title"].as<std::string>();
            response["price"] = result[0]["price"].as<int>();
            response["updated_at"] = result[0]["updated_at"].as<std::string>();

            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        }
               >> [callback](const DrogonDbException &e) {
                   LOG_ERROR << "DB error: " << e.base().what();
                   Json::Value error;
                   error["error"] = "Failed to update product";
                   auto resp = HttpResponse::newHttpJsonResponse(error);
                   resp->setStatusCode(k500InternalServerError);
                   callback(resp);
               };
    };

    executeUpdate();
}

void ProductController::deleteProduct(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        const std::string &product_id)
{
    LOG_INFO << "DELETE /api/admin/products/" << product_id;

    auto dbClient = app().getDbClient();
    if (!dbClient) {
        Json::Value error;
        error["error"] = "Database not configured";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }

    std::string sql = R"(
        UPDATE products
        SET is_active = false, updated_at = CURRENT_TIMESTAMP
        WHERE product_id = $1
        RETURNING title
    )";

    dbClient->execSqlAsync(
            sql,
            [callback, product_id](const Result &result) {
                if (result.empty()) {
                    Json::Value error;
                    error["error"] = "Product not found";
                    auto resp = HttpResponse::newHttpJsonResponse(error);
                    resp->setStatusCode(k404NotFound);
                    callback(resp);
                    return;
                }

                Json::Value response;
                response["success"] = true;
                response["product_id"] = product_id;
                response["message"] = "Product deleted successfully";

                auto resp = HttpResponse::newHttpJsonResponse(response);
                callback(resp);
            },
            [callback](const DrogonDbException &e) {
                LOG_ERROR << "DB error: " << e.base().what();
                Json::Value error;
                error["error"] = "Failed to delete product";
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
            },
            product_id
    );
}