#include "UploadController.h"
#include <drogon/MultiPart.h>
#include <filesystem>
#include <random>

using namespace drogon;

std::string UploadController::generateRandomString(int length) {
    static const char alphanum[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    
    std::string result;
    result.reserve(length);
    for (int i = 0; i < length; ++i) {
        result += alphanum[dis(gen)];
    }
    return result;
}

bool UploadController::isValidImageType(const std::string &contentType) {
    return contentType == "image/jpeg" || 
           contentType == "image/jpg" || 
           contentType == "image/png" || 
           contentType == "image/webp";
}

void UploadController::uploadProductImage(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_INFO << "POST /api/admin/upload/product-image";

    MultiPartParser fileUpload;
    if (fileUpload.parse(req) != 0 || fileUpload.getFiles().empty()) {
        Json::Value error;
        error["error"] = "No file uploaded or invalid multipart data";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    auto &files = fileUpload.getFiles();
    auto &file = files[0];

    // Validate file size (5 MB max)
    const size_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5 MB
    if (file.fileLength() > MAX_FILE_SIZE) {
        Json::Value error;
        error["error"] = "File too large. Maximum size is 5 MB";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    // Validate file type
    std::string contentType = file.getContentType();
    if (!isValidImageType(contentType)) {
        Json::Value error;
        error["error"] = "Invalid file type. Only JPEG, PNG, and WEBP are allowed";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    // Determine file extension
    std::string ext = ".jpg";
    if (contentType == "image/png") {
        ext = ".png";
    } else if (contentType == "image/webp") {
        ext = ".webp";
    }

    // Generate unique filename
    std::string filename = generateRandomString(16) + ext;
    std::string uploadDir = "./uploads/products";
    std::string filePath = uploadDir + "/" + filename;

    // Create uploads directory if it doesn't exist
    try {
        std::filesystem::create_directories(uploadDir);
    } catch (const std::exception &e) {
        LOG_ERROR << "Failed to create upload directory: " << e.what();
        Json::Value error;
        error["error"] = "Failed to create upload directory";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }

    // Save file
    try {
        file.saveAs(filePath);
        LOG_INFO << "File saved: " << filePath;
    } catch (const std::exception &e) {
        LOG_ERROR << "Failed to save file: " << e.what();
        Json::Value error;
        error["error"] = "Failed to save file";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        return;
    }

    // Return success response
    Json::Value response;
    response["success"] = true;
    response["image_url"] = "/uploads/products/" + filename;
    response["filename"] = filename;
    response["size"] = static_cast<Json::Int64>(file.fileLength());

    auto resp = HttpResponse::newHttpJsonResponse(response);
    resp->setStatusCode(k201Created);
    callback(resp);
}
