#include "ShopController.h"
#include <drogon/orm/DbClient.h>
#include <algorithm>
#include <cctype>

using namespace drogon;
using namespace drogon::orm;

std::string generateShopToken(const std::string& title, int64_t owner_tg_id) {
    std::string token = title;
    std::transform(token.begin(), token.end(), token.begin(), ::tolower);

    std::replace_if(token.begin(), token.end(),
                    [](char c) { return !std::isalnum(static_cast<unsigned char>(c)); }, '_');

    auto new_end = std::unique(token.begin(), token.end(),
                               [](char a, char b) { return a == '_' && b == '_'; });
    token.erase(new_end, token.end());

    if (!token.empty() && token[0] == '_') token.erase(0, 1);
    if (!token.empty() && token.back() == '_') token.pop_back();

    std::string suffix = std::to_string(owner_tg_id % 10000);
    token = token + "_" + suffix;

    if (token.length() > 32) {
        token = token.substr(0, 32);
    }

    return token;
}

bool isValidShopToken(const std::string& token) {
    if (token.empty() || token.length() < 3 || token.length() > 32) {
        return false;
    }

    for (char c : token) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return false;
        }
    }

    if (token[0] == '_' || token.back() == '_') {
        return false;
    }

    return true;
}

void ShopController::getShopByToken(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        const std::string &shop_token)
{
    LOG_INFO << "GET /api/shops/token/" << shop_token;

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
        SELECT id, title, description, currency, created_at, owner_tg_id
        FROM shops
        WHERE shop_token = $1
    )";

    dbClient->execSqlAsync(
            sql,
            [callback, shop_token, dbClient](const Result &result) {
                if (result.empty()) {
                    Json::Value error;
                    error["error"] = "Shop not found";
                    error["code"] = "SHOP_NOT_FOUND";
                    auto resp = HttpResponse::newHttpJsonResponse(error);
                    resp->setStatusCode(k404NotFound);
                    callback(resp);
                    return;
                }

                auto shopPtr = std::make_shared<Json::Value>();
                const auto &row = result[0];

                (*shopPtr)["id"] = row["id"].as<int>();
                (*shopPtr)["title"] = row["title"].as<std::string>();
                (*shopPtr)["description"] = row["description"].isNull() ? "" : row["description"].as<std::string>();
                (*shopPtr)["currency"] = row["currency"].as<std::string>();
                (*shopPtr)["shop_token"] = shop_token;
                (*shopPtr)["created_at"] = row["created_at"].as<std::string>();

                int shop_id = row["id"].as<int>();

                dbClient->execSqlAsync(
                        "SELECT COUNT(*) as count FROM products WHERE shop_id = $1 AND is_active = true",
                        [shopPtr, callback](const Result &countResult) {
                            (*shopPtr)["products_count"] = countResult[0]["count"].as<int>();

                            auto resp = HttpResponse::newHttpJsonResponse(*shopPtr);
                            resp->addHeader("Access-Control-Allow-Origin", "*");
                            callback(resp);
                        },
                        [callback](const DrogonDbException &e) {
                            LOG_ERROR << "DB error (count): " << e.base().what();
                            Json::Value error;
                            error["error"] = "Database error";
                            auto resp = HttpResponse::newHttpJsonResponse(error);
                            resp->setStatusCode(k500InternalServerError);
                            callback(resp);
                        },
                        shop_id
                );
            },
            [callback](const DrogonDbException &e) {
                LOG_ERROR << "DB error: " << e.base().what();
                Json::Value error;
                error["error"] = "Database error";
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
            },
            shop_token
    );
}

void ShopController::createShop(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_INFO << "POST /api/shops";

    auto json = req->getJsonObject();
    if (!json || !json->isMember("owner_tg_id") || !json->isMember("title")) {
        Json::Value error;
        error["error"] = "Missing required fields: owner_tg_id and title";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    int64_t owner_tg_id = (*json)["owner_tg_id"].asInt64();
    std::string title = (*json)["title"].asString();
    std::string description = json->get("description", "").asString();
    std::string currency = json->get("currency", "RUB").asString();

    std::string shop_token;

    if (json->isMember("shop_token") && !(*json)["shop_token"].asString().empty()) {
        shop_token = (*json)["shop_token"].asString();

        if (!isValidShopToken(shop_token)) {
            Json::Value error;
            error["error"] = "Invalid shop_token format";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
    } else {
        shop_token = generateShopToken(title, owner_tg_id);
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

    std::string checkSql = "SELECT COUNT(*) as count FROM shops WHERE shop_token = $1";

    dbClient->execSqlAsync(
            checkSql,
            [dbClient, owner_tg_id, title, description, currency, shop_token, callback](const Result &checkResult) {
                int count = checkResult[0]["count"].as<int>();

                std::string final_token = shop_token;

                if (count > 0) {
                    final_token = shop_token + "_" + std::to_string(std::time(nullptr) % 1000);
                }

                std::string sql = R"(
                INSERT INTO shops (shop_token, owner_tg_id, title, description, currency)
                VALUES ($1, $2, $3, $4, $5)
                RETURNING id, created_at
            )";

                dbClient->execSqlAsync(
                        sql,
                        [callback, final_token, title, shop_token](const Result &result) {
                            Json::Value response;
                            response["success"] = true;
                            response["shop"] = Json::objectValue;
                            response["shop"]["id"] = result[0]["id"].as<int>();
                            response["shop"]["shop_token"] = final_token;
                            response["shop"]["title"] = title;
                            response["shop"]["created_at"] = result[0]["created_at"].as<std::string>();

                            if (final_token != shop_token) {
                                response["message"] = "Shop created (token modified for uniqueness)";
                            } else {
                                response["message"] = "Shop created successfully";
                            }

                            auto resp = HttpResponse::newHttpJsonResponse(response);
                            resp->setStatusCode(k201Created);
                            callback(resp);
                        },
                        [callback](const DrogonDbException &e) {
                            LOG_ERROR << "DB error: " << e.base().what();
                            Json::Value error;
                            error["error"] = "Failed to create shop";
                            auto resp = HttpResponse::newHttpJsonResponse(error);
                            resp->setStatusCode(k500InternalServerError);
                            callback(resp);
                        },
                        final_token, owner_tg_id, title, description, currency
                );
            },
            [callback](const DrogonDbException &e) {
                LOG_ERROR << "DB error: " << e.base().what();
                Json::Value error;
                error["error"] = "Database error";
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
            },
            shop_token
    );
}

void ShopController::updateShop(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        const std::string &shop_token)
{
    LOG_INFO << "PUT /api/shops/" << shop_token;

    auto json = req->getJsonObject();
    if (!json) {
        Json::Value error;
        error["error"] = "Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    // Проверка владельца
    std::string auth_header = req->getHeader("X-Owner-TG-ID");
    if (auth_header.empty()) {
        auth_header = req->getParameter("owner_tg_id");
    }

    if (auth_header.empty()) {
        Json::Value error;
        error["error"] = "Authentication required";
        error["code"] = "AUTH_REQUIRED";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k401Unauthorized);
        callback(resp);
        return;
    }

    int64_t owner_tg_id = 0;
    try {
        owner_tg_id = std::stoll(auth_header);
    } catch (...) {
        Json::Value error;
        error["error"] = "Invalid owner_tg_id";
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

    std::string checkSql = "SELECT id FROM shops WHERE shop_token = $1 AND owner_tg_id = $2";

    dbClient->execSqlAsync(
            checkSql,
            [dbClient, json, shop_token, callback](const Result &ownerResult) {
                if (ownerResult.empty()) {
                    Json::Value error;
                    error["error"] = "Access denied";
                    error["code"] = "FORBIDDEN";
                    auto resp = HttpResponse::newHttpJsonResponse(error);
                    resp->setStatusCode(k403Forbidden);
                    callback(resp);
                    return;
                }

                // ✅ Строим SQL вручную
                std::vector<std::string> setClauses;
                int paramIndex = 1;

                std::string title_val, description_val, currency_val;
                bool has_title = false, has_description = false, has_currency = false;

                if (json->isMember("title")) {
                    setClauses.push_back("title = $" + std::to_string(paramIndex++));
                    title_val = (*json)["title"].asString();
                    has_title = true;
                }

                if (json->isMember("description")) {
                    setClauses.push_back("description = $" + std::to_string(paramIndex++));
                    description_val = (*json)["description"].asString();
                    has_description = true;
                }

                if (json->isMember("currency")) {
                    setClauses.push_back("currency = $" + std::to_string(paramIndex++));
                    currency_val = (*json)["currency"].asString();
                    has_currency = true;
                }

                if (setClauses.empty()) {
                    Json::Value error;
                    error["error"] = "No fields to update";
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

                std::string sql = "UPDATE shops SET " + setClause +
                                  " WHERE shop_token = $" + std::to_string(paramIndex) +
                                  " RETURNING title, currency, updated_at";

                // ✅ Выполняем UPDATE с правильными параметрами
                auto binder = *dbClient << sql;

                if (has_title) binder << title_val;
                if (has_description) binder << description_val;
                if (has_currency) binder << currency_val;

                binder << shop_token;

                binder >> [callback, shop_token](const Result &result) {
                    if (result.empty()) {
                        Json::Value error;
                        error["error"] = "Shop not found";
                        auto resp = HttpResponse::newHttpJsonResponse(error);
                        resp->setStatusCode(k404NotFound);
                        callback(resp);
                        return;
                    }

                    Json::Value response;
                    response["success"] = true;
                    response["shop_token"] = shop_token;
                    response["title"] = result[0]["title"].as<std::string>();
                    response["currency"] = result[0]["currency"].as<std::string>();
                    response["updated_at"] = result[0]["updated_at"].as<std::string>();

                    auto resp = HttpResponse::newHttpJsonResponse(response);
                    callback(resp);
                }
                       >> [callback](const DrogonDbException &e) {
                           Json::Value error;
                           error["error"] = "Failed to update shop";
                           auto resp = HttpResponse::newHttpJsonResponse(error);
                           resp->setStatusCode(k500InternalServerError);
                           callback(resp);
                       };
            },
            [callback](const DrogonDbException &e) {
                Json::Value error;
                error["error"] = "Database error";
                auto resp = HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
            },
            shop_token, owner_tg_id
    );
}