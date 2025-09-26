#pragma once
#include "layer.h"


class AppLayer : koala::core::Layer {
public:
    void OnAttach() override;
    void OnDetach() override;
    void OnEvent(koala::core::Event &e) override;
    void OnRender() override;
    void OnUpdate(float dt) override;
};
