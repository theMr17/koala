#pragma once
#include <string>
#include "http_request.h"
#include "http_response.h"

namespace koala::storage {

    /**
     * @brief Save an HTTP request/response pair in a Postman-like hierarchy.
     *
     * Example folder structure:
     * project_root/
     *   └── Posts/
     *         └── GetPost.reqres
     *
     * @param projectRoot Root folder for storage.
     * @param folderName  Subfolder name.
     * @param fileName    File name (e.g., "GetPost.reqres").
     * @param req         HTTP request object.
     * @param resp        HTTP response object.
     */
    void saveReqRes(
        const std::string& projectRoot,
        const std::string& folderName,
        const std::string& fileName,
        const network::HttpRequest& req,
        const network::HttpResponse& resp
    );

} // namespace koala::storage
