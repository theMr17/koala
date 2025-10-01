#include "storage.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace koala::storage {

    namespace {
        std::string serializeRequest(const network::HttpRequest& req) {
            std::ostringstream oss;
            oss << req.method << " " << req.url << "\n";
            for (const auto& [key, value] : req.headers) {
                oss << key << ": " << value << "\n";
            }
            if (!req.body.empty()) {
                oss << "\n" << req.body << "\n";
            }
            if (req.timeout_ms) {
                oss << "Timeout: " << *req.timeout_ms << " ms\n";
            }
            return oss.str();
        }

        std::string serializeResponse(const network::HttpResponse& resp) {
            std::ostringstream oss;
            oss << "HTTP/1.1 " << resp.status_code;
            if (!resp.status_text.empty()) oss << " " << resp.status_text;
            oss << "\n";
            for (const auto& [key, value] : resp.headers) {
                oss << key << ": " << value << "\n";
            }
            if (!resp.body.empty()) {
                oss << "\n" << resp.body << "\n";
            }
            return oss.str();
        }
    } // namespace

    void saveReqRes(
        const std::string& projectRoot,
        const std::string& folderName,
        const std::string& fileName,
        const network::HttpRequest& req,
        const network::HttpResponse& resp
    ) {
        namespace fs = std::filesystem;

        // Ensure project root and subfolder exist
        fs::path folderPath = fs::path(projectRoot) / folderName;
        fs::create_directories(folderPath);

        // Write combined request + response into file
        fs::path filePath = folderPath / fileName;
        std::ofstream out(filePath);
        out << "### REQUEST ###\n";
        out << serializeRequest(req) << "\n";
        out << "### RESPONSE ###\n";
        out << serializeResponse(resp) << "\n";
    }

} // namespace koala::storage
