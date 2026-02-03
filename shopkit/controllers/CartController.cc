#include "CartController.h"
#include <drogon/orm/DbClient.h>
#include <json/writer.h>
#include <sstream>

using namespace drogon;
using namespace drogon::orm;

void CartController::saveCart(const HttpRequestPtr &req,
                               std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_INFO << "POST /api/cart";

    auto json = req->getJsonObject();
    if (!json) {
        Json::Value error;
        error["error"] = "Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    // Validate required fields
    if (!json->isMember("telegram_user_id") || !json->isMember("shop_id") || 
        !json->isMember("items")) {
        Json::Value error;
        error["error"] = "Missing required fields: telegram_user_id, shop_id, items";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    int64_t telegram_user_id = (*json)["telegram_user_id"].asInt64();
    int shop_id = (*json)["shop_id"].asInt();
    Json::Value items = (*json)["items"];

    // Convert items to JSON string
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
        INSERT INTO carts (telegram_user_id, shop_id, items)
        VALUES ($1, $2, $3::jsonb)
        ON CONFLICT (telegram_user_id, shop_id)
        DO UPDATE SET items = EXCLUDED.items, updated_at = NOW()
        RETURNING updated_at
    )";

    dbClient->execSqlAsync(
        sql,
        [callback](const Result &result) {
            Json::Value response;
            response["success"] = true;
            response["updated_at"] = result[0]["updated_at"].as<std::string>();
            
            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        },
        [callback](const DrogonDbException &e) {
            LOG_ERROR << "DB error: " << e.base().what();
            Json::Value error;
            error["error"] = "Failed to save cart";
            error["details"] = e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        },
        telegram_user_id, shop_id, items_json
    );
}

void CartController::getCart(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback,
                              int64_t telegram_user_id)
{
    LOG_INFO << "GET /api/cart/" << telegram_user_id;

    std::string shop_id_param = req->getParameter("shop_id");
    if (shop_id_param.empty()) {
        Json::Value error;
        error["error"] = "Missing required parameter: shop_id";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    int shop_id = 0;
    try {
        shop_id = std::stoi(shop_id_param);
    } catch (const std::exception &) {
        Json::Value error;
        error["error"] = "Invalid shop_id parameter";
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
        SELECT items, updated_at
        FROM carts
        WHERE telegram_user_id = $1 AND shop_id = $2
    )";

    dbClient->execSqlAsync(
        sql,
        [callback, telegram_user_id, shop_id](const Result &result) {
            Json::Value response;
            response["telegram_user_id"] = telegram_user_id;
            response["shop_id"] = shop_id;

            if (result.empty()) {
                response["items"] = Json::arrayValue;
                auto resp = HttpResponse::newHttpJsonResponse(response);
                callback(resp);
                return;
            }

            // Parse items JSON
            Json::CharReaderBuilder reader;
            Json::Value items;
            std::string items_str = result[0]["items"].as<std::string>();
            std::string errs;
            std::istringstream items_stream(items_str);
            
            if (Json::parseFromStream(reader, items_stream, &items, &errs)) {
                response["items"] = items;
            } else {
                response["items"] = Json::arrayValue;
            }

            response["updated_at"] = result[0]["updated_at"].as<std::string>();

            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        },
        [callback](const DrogonDbException &e) {
            LOG_ERROR << "DB error: " << e.base().what();
            Json::Value error;
            error["error"] = "Failed to get cart";
            error["details"] = e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        },
        telegram_user_id, shop_id
    );
}

void CartController::clearCart(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback,
                                int64_t telegram_user_id)
{
    LOG_INFO << "DELETE /api/cart/" << telegram_user_id;

    std::string shop_id_param = req->getParameter("shop_id");
    if (shop_id_param.empty()) {
        Json::Value error;
        error["error"] = "Missing required parameter: shop_id";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    int shop_id = 0;
    try {
        shop_id = std::stoi(shop_id_param);
    } catch (const std::exception &) {
        Json::Value error;
        error["error"] = "Invalid shop_id parameter";
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

    std::string sql = "DELETE FROM carts WHERE telegram_user_id = $1 AND shop_id = $2";

    dbClient->execSqlAsync(
        sql,
        [callback](const Result &result) {
            Json::Value response;
            response["success"] = true;
            
            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        },
        [callback](const DrogonDbException &e) {
            LOG_ERROR << "DB error: " << e.base().what();
            Json::Value error;
            error["error"] = "Failed to clear cart";
            error["details"] = e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        },
        telegram_user_id, shop_id
    );
}
