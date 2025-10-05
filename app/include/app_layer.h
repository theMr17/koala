#pragma once
#include <future>
#include <optional>
#include <string>
#include <vector>
#include "layer.h"
#include "http_client.h"
#include "storage.h"

// A simple GUI layer that demonstrates async HTTP using ImGui and provides Postman-like collections.
class AppLayer : public koala::core::Layer {
public:
    void OnAttach() override;
    void OnDetach() override;
    void OnEvent(koala::core::Event &e) override;
    void OnRender() override;
    void OnUpdate(float dt) override;

private:
    // UI state for composing a request
    std::string method_ = "GET";
    static constexpr int kUrlBufSize = 1024;
    static constexpr int kBodyBufSize = 8192;
    char urlBuf_[kUrlBufSize] = {0};
    char bodyBuf_[kBodyBufSize] = {0};

    // Result state
    bool inFlight_ = false;
    int lastStatus_ = 0;
    std::string lastBody_;
    std::string lastError_;

    // Keep full last request/response for saving
    std::optional<koala::network::HttpRequest> lastReq_;
    std::optional<koala::network::HttpResponse> lastResp_;

    // Collections state
    std::string projectRoot_ = "app\\project_storage"; // relative to CWD
    std::vector<std::string> collections_;
    std::vector<std::string> requestsInSelected_;
    int selectedCollectionIdx_ = -1;
    int selectedRequestIdx_ = -1;
    static constexpr int kNameBufSize = 256;
    char newCollectionBuf_[kNameBufSize] = {0};
    char newRequestNameBuf_[kNameBufSize] = {0};

    // Selection and editing state
    std::string currentLoadedCollection_;
    std::string currentLoadedFile_; // file name with extension
    bool dirty_ = false;

    // Async handle
    std::optional<std::future<koala::network::NetResult>> future_;

    void startRequest();

    // Storage helpers
    void refreshCollections();
    void refreshRequestsForSelected();
    void saveCurrentToStorage();

    // Loading and saving helpers
    void loadRequestFile(const std::string& collection, const std::string& fileName);
    void overwriteSaveCurrent();
};
