#pragma once
#include <future>
#include <optional>
#include <string>
#include "layer.h"
#include "http_client.h"

// A simple GUI layer that demonstrates async HTTP using ImGui.
class AppLayer : public koala::core::Layer {
public:
    void OnAttach() override;
    void OnDetach() override;
    void OnEvent(koala::core::Event &e) override;
    void OnRender() override;
    void OnUpdate(float dt) override;

private:
    // UI state
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

    // Async handle
    std::optional<std::future<koala::network::NetResult>> future_;

    void startRequest();
};
