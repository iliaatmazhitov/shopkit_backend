#include "OrderController.h"
#include <drogon/orm/DbClient.h>
#include <algorithm>
#include <memory>

using namespace drogon;
using namespace drogon::orm;

void OrderController::createOrder(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_INFO << "POST /api/orders";

    auto json = req->getJsonObject();
    if (!json || !json->isMember("shop_id") || !json->isMember("items") || !json->isMember("total_price")) {
        Json::Value error;
        error["error"] = "Missing required fields: shop_id, items, total_price";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    int shop_id = (*json)["shop_id"].asInt();
    Json::Value items = (*json)["items"];
    int total_price = (*json)["total_price"].asInt();
    std::string customer_name = json->get("customer_name", "").asString();
    std::string customer_phone = json->get("customer_phone", "").asString();
    std::string currency = json->get("currency", "RUB").asString();
    std::string notes = json->get("notes", "").asString();

    auto dbClient = app().getDbClient();
    if (!dbClient) {
        Json::Value error;
        error["error"] = "Database not configured";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }

    // Convert items to JSONB string
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string items_json = Json::writeString(builder, items);

    std::string sql = R"(
        INSERT INTO orders (shop_id, customer_name, customer_phone, items, total_price, currency, notes, status)
        VALUES ($1, $2, $3, $4::jsonb, $5, $6, $7, 'pending')
        RETURNING order_id, created_at
    )";

    dbClient->execSqlAsync(
        sql,
        [callback, total_price, currency](const Result &result) {
            Json::Value response;
            response["success"] = true;
            response["order"] = Json::objectValue;
            response["order"]["order_id"] = result[0]["order_id"].as<std::string>();
            response["order"]["total_price"] = total_price;
            response["order"]["currency"] = currency;
            response["order"]["status"] = "pending";
            response["order"]["created_at"] = result[0]["created_at"].as<std::string>();

            auto resp = HttpResponse::newHttpJsonResponse(response);
            resp->setStatusCode(k201Created);
            callback(resp);
        },
        [callback](const DrogonDbException &e) {
            LOG_ERROR << "DB error: " << e.base().what();
            Json::Value error;
            error["error"] = "Failed to create order";
            error["code"] = "DB_ERROR";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        },
        shop_id, customer_name, customer_phone, items_json, total_price, currency, notes
    );
}

void OrderController::getOrder(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    const std::string &order_id)
{
    LOG_INFO << "GET /api/orders/" << order_id;

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
        SELECT o.*, s.title as shop_title, s.shop_token
        FROM orders o
        LEFT JOIN shops s ON o.shop_id = s.id
        WHERE o.order_id = $1
    )";

    dbClient->execSqlAsync(
        sql,
        [callback](const Result &result) {
            if (result.empty()) {
                Json::Value error;
                error["error"] = "Order not found";
                error["code"] = "ORDER_NOT_FOUND";
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k404NotFound);
                callback(resp);
                return;
            }

            const auto &row = result[0];
            Json::Value order;
            order["order_id"] = row["order_id"].as<std::string>();
            order["shop_id"] = row["shop_id"].as<int>();
            order["customer_name"] = row["customer_name"].isNull() ? "" : row["customer_name"].as<std::string>();
            order["customer_phone"] = row["customer_phone"].isNull() ? "" : row["customer_phone"].as<std::string>();
            order["total_price"] = row["total_price"].as<int>();
            order["currency"] = row["currency"].as<std::string>();
            order["status"] = row["status"].as<std::string>();
            order["created_at"] = row["created_at"].as<std::string>();

            // Parse JSONB items
            std::string items_str = row["items"].as<std::string>();
            Json::CharReaderBuilder builder;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            Json::Value items;
            std::string errors;
            bool parsingSuccessful = reader->parse(
                items_str.c_str(),
                items_str.c_str() + items_str.size(),
                &items,
                &errors
            );
            
            if (!parsingSuccessful) {
                LOG_ERROR << "Failed to parse items JSON: " << errors;
                items = Json::arrayValue; // fallback to empty array
            }
            order["items"] = items;

            order["shop"] = Json::objectValue;
            order["shop"]["title"] = row["shop_title"].as<std::string>();
            order["shop"]["shop_token"] = row["shop_token"].as<std::string>();

            auto resp = HttpResponse::newHttpJsonResponse(order);
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
        order_id
    );
}

void OrderController::getShopOrders(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    int shop_id)
{
    LOG_INFO << "GET /api/admin/orders/" << shop_id;

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
        SELECT order_id, customer_name, total_price, currency, status, created_at
        FROM orders
        WHERE shop_id = $1
        ORDER BY created_at DESC
    )";

    dbClient->execSqlAsync(
        sql,
        [callback](const Result &result) {
            Json::Value response(Json::arrayValue);

            for (const auto &row : result) {
                Json::Value order;
                order["order_id"] = row["order_id"].as<std::string>();
                order["customer_name"] = row["customer_name"].isNull() ? "" : row["customer_name"].as<std::string>();
                order["total_price"] = row["total_price"].as<int>();
                order["currency"] = row["currency"].as<std::string>();
                order["status"] = row["status"].as<std::string>();
                order["created_at"] = row["created_at"].as<std::string>();
                response.append(order);
            }

            auto resp = HttpResponse::newHttpJsonResponse(response);
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
        shop_id
    );
}

void OrderController::updateOrderStatus(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    const std::string &order_id)
{
    LOG_INFO << "PUT /api/admin/orders/" << order_id;

    auto json = req->getJsonObject();
    if (!json || !json->isMember("status")) {
        Json::Value error;
        error["error"] = "Missing required field: status";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    std::string status = (*json)["status"].asString();

    // Validate status value
    const std::vector<std::string> validStatuses = {"pending", "processing", "completed", "cancelled"};
    if (std::find(validStatuses.begin(), validStatuses.end(), status) == validStatuses.end()) {
        Json::Value error;
        error["error"] = "Invalid status value. Must be one of: pending, processing, completed, cancelled";
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

    std::string sql = R"(
        UPDATE orders
        SET status = $1, updated_at = CURRENT_TIMESTAMP
        WHERE order_id = $2
        RETURNING order_id, status, updated_at
    )";

    dbClient->execSqlAsync(
        sql,
        [callback](const Result &result) {
            if (result.empty()) {
                Json::Value error;
                error["error"] = "Order not found";
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k404NotFound);
                callback(resp);
                return;
            }

            Json::Value response;
            response["success"] = true;
            response["order_id"] = result[0]["order_id"].as<std::string>();
            response["status"] = result[0]["status"].as<std::string>();
            response["updated_at"] = result[0]["updated_at"].as<std::string>();

            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        },
        [callback](const DrogonDbException &e) {
            LOG_ERROR << "DB error: " << e.base().what();
            Json::Value error;
            error["error"] = "Failed to update order";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        },
        status, order_id
    );
}
