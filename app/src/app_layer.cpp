#include "app_layer.h"
#include "imgui.h"
#include <chrono>
#include <cstring>
#include <cstdio>
#include <cfloat>

void AppLayer::OnAttach() {
    // Set a default URL into the fixed buffer
    const char* def = "https://jsonplaceholder.typicode.com/posts/1";
#ifdef _MSC_VER
    strcpy_s(urlBuf_, kUrlBufSize, def);
#else
    std::snprintf(urlBuf_, kUrlBufSize, "%s", def);
#endif

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
            lastBody_ = std::move(res.response.body);
            if (res.error) {
                lastError_ = res.error.message();
            }
        }
    }
}

void AppLayer::OnRender() {
    ImGui::Begin("HTTP Client");

    // Method selection (simple)
    const char* methods[] = {"GET", "POST", "PUT", "DELETE"};
    int current = 0;
    for (int i = 0; i < 4; ++i) if (method_ == methods[i]) current = i;
    if (ImGui::BeginCombo("Method", methods[current])) {
        for (int i = 0; i < 4; ++i) {
            bool selected = (current == i);
            if (ImGui::Selectable(methods[i], selected)) {
                current = i; method_ = methods[i];
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // URL input
    ImGui::InputText("URL", urlBuf_, kUrlBufSize);

    if (method_ != std::string("GET")) {
        ImGui::InputTextMultiline("Body", bodyBuf_, kBodyBufSize, ImVec2(-FLT_MIN, 120));
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

    future_ = koala::network::send_async_future(std::move(req));
    inFlight_ = true;
}
