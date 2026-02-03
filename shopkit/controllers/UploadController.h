#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class UploadController : public HttpController<UploadController>
{
public:
    METHOD_LIST_BEGIN

        ADD_METHOD_TO(UploadController::uploadProductImage,
                      "/api/admin/upload/product-image",
                      Post);

    METHOD_LIST_END

    void uploadProductImage(const HttpRequestPtr &req,
                           std::function<void(const HttpResponsePtr &)> &&callback);

private:
    std::string generateRandomString(int length);
    bool isValidImageType(const std::string &contentType);
};
