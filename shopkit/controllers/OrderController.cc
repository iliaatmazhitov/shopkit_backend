#include "OrderController.h"
#include <drogon/orm/DbClient.h>
#include <json/writer.h>
#include <sstream>

using namespace drogon;
using namespace drogon::orm;

void OrderController::createOrder(const HttpRequestPtr &req,
                                  std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_INFO << "POST /api/orders";

    auto json = req->getJsonObject();
    if (!json) {
        Json::Value error;
        error["error"] = "Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    if (!json->isMember("shop_id") || !json->isMember("items") || 
        !json->isMember("total_price") || !json->isMember("currency")) {
        Json::Value error;
        error["error"] = "Missing required fields: shop_id, items, total_price, currency";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    int shop_id = (*json)["shop_id"].asInt();
    int64_t telegram_user_id = json->get("telegram_user_id", 0).asInt64();
    std::string customer_name = json->get("customer_name", "").asString();
    std::string customer_phone = json->get("customer_phone", "").asString();
    Json::Value items = (*json)["items"];
    int total_price = (*json)["total_price"].asInt();
    std::string currency = (*json)["currency"].asString();
    std::string status = json->get("status", "pending").asString();
    std::string payment_method = json->get("payment_method", "").asString();
    std::string notes = json->get("notes", "").asString();

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string items_json = Json::writeString(builder, items);

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
        INSERT INTO orders (shop_id, telegram_user_id, customer_name, customer_phone, 
                          items, total_price, currency, status, payment_method, notes)
        VALUES ($1, $2, $3, $4, $5::jsonb, $6, $7, $8, $9, $10)
        RETURNING order_id, created_at
    )";

    dbClient->execSqlAsync(
        sql,
        [callback, shop_id, total_price, currency, items, status](const Result &result) {
            Json::Value response;
            response["success"] = true;
            
            Json::Value order;
            order["order_id"] = result[0]["order_id"].as<std::string>();
            order["shop_id"] = shop_id;
            order["total_price"] = total_price;
            order["currency"] = currency;
            order["status"] = status;
            order["items"] = items;
            order["created_at"] = result[0]["created_at"].as<std::string>();
            
            response["order"] = order;
            
            auto resp = HttpResponse::newHttpJsonResponse(response);
            resp->setStatusCode(k201Created);
            callback(resp);
        },
        [callback](const DrogonDbException &e) {
            LOG_ERROR << "DB error: " << e.base().what();
            Json::Value error;
            error["error"] = "Failed to create order";
            error["details"] = e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        },
        shop_id, telegram_user_id, customer_name, customer_phone, 
        items_json, total_price, currency, status, payment_method, notes
    );
}

void OrderController::getUserOrders(const HttpRequestPtr &req,
                                    std::function<void(const HttpResponsePtr &)> &&callback,
                                    int64_t telegram_user_id)
{
    LOG_INFO << "GET /api/user/orders/" << telegram_user_id;

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
        SELECT 
            o.order_id,
            o.shop_id,
            s.title as shop_title,
            s.shop_token,
            o.total_price,
            o.currency,
            o.status,
            o.items,
            o.created_at
        FROM orders o
        JOIN shops s ON o.shop_id = s.id
        WHERE o.telegram_user_id = $1
        ORDER BY o.created_at DESC
    )";

    dbClient->execSqlAsync(
        sql,
        [callback](const Result &result) {
            Json::Value response(Json::arrayValue);

            for (const auto &row : result) {
                Json::Value order;
                order["order_id"] = row["order_id"].as<std::string>();
                order["shop_id"] = row["shop_id"].as<int>();
                order["shop_title"] = row["shop_title"].as<std::string>();
                order["shop_token"] = row["shop_token"].as<std::string>();
                order["total_price"] = row["total_price"].as<int>();
                order["currency"] = row["currency"].as<std::string>();
                order["status"] = row["status"].as<std::string>();
                order["created_at"] = row["created_at"].as<std::string>();

                Json::CharReaderBuilder reader;
                Json::Value items;
                std::string items_str = row["items"].as<std::string>();
                std::string errs;
                std::istringstream items_stream(items_str);
                
                if (Json::parseFromStream(reader, items_stream, &items, &errs)) {
                    order["items"] = items;
                } else {
                    order["items"] = Json::arrayValue;
                }

                response.append(order);
            }
            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        },
        [callback](const DrogonDbException &e) {
            LOG_ERROR << "DB error: " << e.base().what();
            Json::Value error;
            error["error"] = "Failed to get user orders";
            error["details"] = e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        },
        telegram_user_id
    );
}
