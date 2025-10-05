#include "app_layer.h"
#include "imgui.h"
#include <chrono>
#include <cstring>
#include <cstdio>
#include <cfloat>
#include <filesystem>
#include <fstream>

using namespace std::string_literals;

void AppLayer::OnAttach() {
    // Set a default URL into the fixed buffer
    const char* def = "https://jsonplaceholder.typicode.com/posts/1";
#ifdef _MSC_VER
    strcpy_s(urlBuf_, kUrlBufSize, def);
#else
    std::snprintf(urlBuf_, kUrlBufSize, "%s", def);
#endif

    // Prepare storage root folder
    try {
        std::filesystem::create_directories(projectRoot_);
    } catch (...) {}
    refreshCollections();

    // Kick off an initial request so the UI shows something when it opens
    startRequest();
}

void AppLayer::OnDetach() {}

void AppLayer::OnEvent(koala::core::Event &e) { (void)e; }

void AppLayer::OnUpdate(float) {
    // Poll future without blocking the frame
    if (inFlight_ && future_.has_value() && future_->valid()) {
        using namespace std::chrono_literals;
        if (future_->wait_for(0ms) == std::future_status::ready) {
            auto res = future_->get();
            inFlight_ = false;
            future_.reset();
            lastError_.clear();
            lastStatus_ = res.response.status_code;
            lastBody_ = res.response.body; // keep a copy
            lastResp_ = std::move(res.response);
            if (res.error) {
                lastError_ = res.error.message();
            }
        }
    }
}

void AppLayer::OnRender() {
    // Left panel: Collections and Requests (tree)
    ImGui::Begin("Collections");
    if (ImGui::Button("Refresh")) { refreshCollections(); }
    ImGui::Separator();

    // New collection creation
    ImGui::InputTextWithHint("##newcoll", "New collection name", newCollectionBuf_, kNameBufSize);
    ImGui::SameLine();
    if (ImGui::Button("Add Collection")) {
        if (std::strlen(newCollectionBuf_) > 0) {
            try {
                std::filesystem::create_directories(std::filesystem::path(projectRoot_) / newCollectionBuf_);
            } catch (...) {}
            newCollectionBuf_[0] = '\0';
            refreshCollections();
        }
    }

    // File tree: collections -> requests
    if (ImGui::BeginChild("##tree", ImVec2(-FLT_MIN, 350), true)) {
        for (int i = 0; i < (int)collections_.size(); ++i) {
            const std::string& coll = collections_[i];
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
            bool open = ImGui::TreeNodeEx((coll + "##coll").c_str(), flags, "%s", coll.c_str());
            if (open) {
                // List files in this collection
                try {
                    auto folder = std::filesystem::path(projectRoot_) / coll;
                    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
                        if (!entry.is_regular_file()) continue;
                        std::string fname = entry.path().filename().string();
                        ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
                        bool isSelected = (currentLoadedCollection_ == coll && currentLoadedFile_ == fname);
                        if (isSelected) leafFlags |= ImGuiTreeNodeFlags_Selected;
                        ImGui::TreeNodeEx((fname + "##file").c_str(), leafFlags, "%s", fname.c_str());
                        if (ImGui::IsItemClicked()) {
                            loadRequestFile(coll, fname);
                        }
                    }
                } catch (...) {}
                ImGui::TreePop();
            }
        }
    }
    ImGui::EndChild();

    // Save As current request/response
    ImGui::InputTextWithHint("##newreq", "Save As (file name, e.g., GetPost)", newRequestNameBuf_, kNameBufSize);
    ImGui::SameLine();
    bool canSaveAs = lastReq_.has_value() && lastResp_.has_value() && selectedCollectionIdx_ >= 0 && std::strlen(newRequestNameBuf_) > 0;
    ImGui::BeginDisabled(!canSaveAs);
    if (ImGui::Button("Save As")) {
        saveCurrentToStorage();
    }
    ImGui::EndDisabled();

    // Overwrite Save when dirty and a file is loaded
    ImGui::SameLine();
    bool canOverwrite = dirty_ && !currentLoadedCollection_.empty() && !currentLoadedFile_.empty() && lastReq_.has_value();
    ImGui::BeginDisabled(!canOverwrite);
    if (ImGui::Button("Save")) {
        overwriteSaveCurrent();
    }
    ImGui::EndDisabled();

    ImGui::End();

    // Right panel: HTTP Client editor and response
    ImGui::Begin("HTTP Client");

    // Method selection (simple)
    const char* methods[] = {"GET", "POST", "PUT", "DELETE"};
    int current = 0;
    for (int i = 0; i < 4; ++i) if (method_ == methods[i]) current = i;
    if (ImGui::BeginCombo("Method", methods[current])) {
        for (int i = 0; i < 4; ++i) {
            bool selected = (current == i);
            if (ImGui::Selectable(methods[i], selected)) {
                current = i; method_ = methods[i]; dirty_ = true;
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // URL input
    if (ImGui::InputText("URL", urlBuf_, kUrlBufSize)) { dirty_ = true; }

    if (method_ != std::string("GET")) {
        if (ImGui::InputTextMultiline("Body", bodyBuf_, kBodyBufSize, ImVec2(-FLT_MIN, 120))) { dirty_ = true; }
    }

    ImGui::BeginDisabled(inFlight_);
    if (ImGui::Button("Send")) {
        startRequest();
    }
    ImGui::EndDisabled();
    if (inFlight_) { ImGui::SameLine(); ImGui::TextUnformatted("(in flight...)"); }

    ImGui::Separator();

    if (!lastError_.empty()) {
        ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "Error: %s", lastError_.c_str());
    } else {
        ImGui::Text("Status: %d", lastStatus_);
    }

    ImGui::Separator();

    ImGui::BeginChild("ResponseBody", ImVec2(0, 0), true);
    ImGui::TextUnformatted(lastBody_.c_str());
    ImGui::EndChild();

    ImGui::End();
}

void AppLayer::startRequest() {
    koala::network::HttpRequest req;
    req.method = method_;
    req.url = std::string(urlBuf_);
    if (method_ != std::string("GET")) req.body = std::string(bodyBuf_);
    req.timeout_ms = 15000; // 15s default

    lastStatus_ = 0;
    lastBody_.clear();
    lastError_.clear();

    lastReq_ = req; // keep a copy for saving
    future_ = koala::network::send_async_future(std::move(req));
    inFlight_ = true;
}

void AppLayer::refreshCollections() {
    collections_.clear();
    using std::filesystem::directory_iterator;
    try {
        if (std::filesystem::exists(projectRoot_)) {
            for (const auto& entry : directory_iterator(projectRoot_)) {
                if (entry.is_directory()) {
                    collections_.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (...) {}
    if (selectedCollectionIdx_ >= (int)collections_.size()) selectedCollectionIdx_ = -1;
    refreshRequestsForSelected();
}

void AppLayer::refreshRequestsForSelected() {
    requestsInSelected_.clear();
    if (selectedCollectionIdx_ < 0 || selectedCollectionIdx_ >= (int)collections_.size()) return;
    auto folder = std::filesystem::path(projectRoot_) / collections_[selectedCollectionIdx_];
    try {
        if (std::filesystem::exists(folder)) {
            for (const auto& entry : std::filesystem::directory_iterator(folder)) {
                if (entry.is_regular_file()) {
                    requestsInSelected_.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (...) {}
    if (selectedRequestIdx_ >= (int)requestsInSelected_.size()) selectedRequestIdx_ = -1;
}

void AppLayer::saveCurrentToStorage() {
    if (selectedCollectionIdx_ < 0) return;
    std::string fname = newRequestNameBuf_;
    if (fname.empty()) return;
    if (fname.rfind(".reqres") == std::string::npos) {
        fname += ".reqres";
    }
    // Build request from editor state
    koala::network::HttpRequest req;
    req.method = method_;
    req.url = std::string(urlBuf_);
    if (method_ != std::string("GET")) req.body = std::string(bodyBuf_);
    req.timeout_ms = 15000;

    // Use last response if available, otherwise a stub
    koala::network::HttpResponse resp;
    if (lastResp_.has_value()) {
        resp = *lastResp_;
    } else {
        resp.status_code = lastStatus_;
        resp.body = lastBody_;
    }

    koala::storage::saveReqRes(
        projectRoot_,
        collections_[selectedCollectionIdx_],
        fname,
        req,
        resp
    );
    newRequestNameBuf_[0] = '\0';
    refreshRequestsForSelected();
}


void AppLayer::loadRequestFile(const std::string& collection, const std::string& fileName) {
    // Open file and parse REQUEST section
    std::filesystem::path path = std::filesystem::path(projectRoot_) / collection / fileName;
    std::ifstream in(path);
    if (!in.is_open()) return;
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    const std::string reqMarker = "### REQUEST ###";
    const std::string respMarker = "### RESPONSE ###";
    auto reqPos = content.find(reqMarker);
    if (reqPos == std::string::npos) return;
    reqPos += reqMarker.size();
    // Skip possible newline after marker
    if (reqPos < content.size() && (content[reqPos] == '\r' || content[reqPos] == '\n')) {
        if (content[reqPos] == '\r' && reqPos + 1 < content.size() && content[reqPos + 1] == '\n') reqPos += 2; else reqPos += 1;
    }
    auto respPos = content.find(respMarker, reqPos);
    std::string reqSection = respPos == std::string::npos ? content.substr(reqPos) : content.substr(reqPos, respPos - reqPos);

    // Parse first line: METHOD URL
    std::istringstream iss(reqSection);
    std::string line;
    std::string loadedMethod;
    std::string loadedUrl;
    if (!std::getline(iss, line)) return;
    {
        // Trim CR
        if (!line.empty() && line.back() == '\r') line.pop_back();
        // Split by space: method and url
        std::istringstream ls(line);
        ls >> loadedMethod;
        std::getline(ls, loadedUrl);
        // trim leading spaces
        while (!loadedUrl.empty() && (loadedUrl[0] == ' ' || loadedUrl[0] == '\t')) loadedUrl.erase(loadedUrl.begin());
    }

    // Skip headers until blank line
    std::string body;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) {
            // remaining is body
            std::string rest((std::istreambuf_iterator<char>(iss)), std::istreambuf_iterator<char>());
            body = rest;
            break;
        }
    }

    // Populate UI state
    if (!loadedMethod.empty()) {
        method_ = loadedMethod;
    }
#ifdef _MSC_VER
    strcpy_s(urlBuf_, kUrlBufSize, loadedUrl.c_str());
#else
    std::snprintf(urlBuf_, kUrlBufSize, "%s", loadedUrl.c_str());
#endif
    if (!body.empty()) {
#ifdef _MSC_VER
        strcpy_s(bodyBuf_, kBodyBufSize, body.c_str());
#else
        std::snprintf(bodyBuf_, kBodyBufSize, "%s", body.c_str());
#endif
    } else {
        bodyBuf_[0] = '\0';
    }

    // Update selection state
    currentLoadedCollection_ = collection;
    currentLoadedFile_ = fileName;
    // Update selectedCollectionIdx_ to match loaded collection
    for (int i = 0; i < (int)collections_.size(); ++i) {
        if (collections_[i] == collection) { selectedCollectionIdx_ = i; break; }
    }

    // Update lastReq_ to represent loaded content
    koala::network::HttpRequest req;
    req.method = method_;
    req.url = std::string(urlBuf_);
    if (method_ != std::string("GET")) req.body = std::string(bodyBuf_);
    req.timeout_ms = 15000;
    lastReq_ = req;

    dirty_ = false;
}

void AppLayer::overwriteSaveCurrent() {
    if (currentLoadedCollection_.empty() || currentLoadedFile_.empty()) return;

    // Build request from editor state
    koala::network::HttpRequest req;
    req.method = method_;
    req.url = std::string(urlBuf_);
    if (method_ != std::string("GET")) req.body = std::string(bodyBuf_);
    req.timeout_ms = 15000;

    // Use last response if available, otherwise stub
    koala::network::HttpResponse resp;
    if (lastResp_.has_value()) {
        resp = *lastResp_;
    } else {
        resp.status_code = lastStatus_;
        resp.body = lastBody_;
    }

    koala::storage::saveReqRes(
        projectRoot_,
        currentLoadedCollection_,
        currentLoadedFile_,
        req,
        resp
    );

    dirty_ = false;
    refreshRequestsForSelected();
}
