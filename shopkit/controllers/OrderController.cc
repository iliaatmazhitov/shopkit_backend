#include "OrderController.h"
#include "utils/UuidGenerator.h"
#include <drogon/orm/DbClient.h>

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

    // ✅ Генерируем UUID в C++
    std::string order_id = UuidGenerator::generate();

    auto dbClient = app().getDbClient();
    if (!dbClient) {
        Json::Value error;
        error["error"] = "Database not configured";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string items_json = Json::writeString(builder, items);

    std::string sql = R"(
        INSERT INTO orders (order_id, shop_id, customer_name, customer_phone, items, total_price, currency, notes, status)
        VALUES ($1, $2, $3, $4, $5::jsonb, $6, $7, $8, 'pending')
        RETURNING order_id, created_at
    )";

    dbClient->execSqlAsync(
        sql,
        [callback, order_id, total_price, currency](const Result &result) {
            Json::Value response;
            response["success"] = true;
            response["order"] = Json::objectValue;
            response["order"]["order_id"] = order_id;
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
        order_id, shop_id, customer_name, customer_phone, items_json, total_price, currency, notes
    );
}

void OrderController::getOrders(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    int shop_id)
{
    LOG_INFO << "GET /api/orders/" << shop_id;

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
        SELECT order_id, customer_name, customer_phone, total_price, currency, status, created_at
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
                order["customer_phone"] = row["customer_phone"].isNull() ? "" : row["customer_phone"].as<std::string>();
                order["total_price"] = row["total_price"].as<int>();
                order["currency"] = row["currency"].as<std::string>();
                order["status"] = row["status"].as<std::string>();
                order["created_at"] = row["created_at"].as<std::string>();
                response.append(order);
            }

            auto resp = HttpResponse::newHttpJsonResponse(response);
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
        shop_id
    );
}

void OrderController::getOrderDetail(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    const std::string &order_id)
{
    LOG_INFO << "GET /api/orders/detail/" << order_id;

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
            order["customer_name"] = row["customer_name"].isNull() ? "" : row["customer_name"].as<std::string>();
            order["customer_phone"] = row["customer_phone"].isNull() ? "" : row["customer_phone"].as<std::string>();
            order["total_price"] = row["total_price"].as<int>();
            order["currency"] = row["currency"].as<std::string>();
            order["status"] = row["status"].as<std::string>();
            order["notes"] = row["notes"].isNull() ? "" : row["notes"].as<std::string>();
            order["created_at"] = row["created_at"].as<std::string>();

            // Parse items JSONB
            std::string items_str = row["items"].as<std::string>();
            Json::CharReaderBuilder readerBuilder;
            Json::Value items;
            std::istringstream iss(items_str);
            std::string errs;
            if (Json::parseFromStream(readerBuilder, iss, &items, &errs)) {
                order["items"] = items;
            } else {
                order["items"] = Json::arrayValue;
            }

            order["shop"] = Json::objectValue;
            order["shop"]["id"] = row["shop_id"].as<int>();
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

void OrderController::updateOrderStatus(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    const std::string &order_id)
{
    LOG_INFO << "PUT /api/admin/orders/" << order_id << "/status";

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
        [callback, order_id](const Result &result) {
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
            response["order_id"] = order_id;
            response["status"] = result[0]["status"].as<std::string>();
            response["updated_at"] = result[0]["updated_at"].as<std::string>();

            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        },
        [callback](const DrogonDbException &e) {
            LOG_ERROR << "DB error: " << e.base().what();
            Json::Value error;
            error["error"] = "Failed to update order status";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        },
        status, order_id
    );
}
