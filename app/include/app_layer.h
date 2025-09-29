#pragma once
#include "layer.h"


/**
 * Application layer that receives lifecycle, update, render, and event callbacks.
 */

/**
 * Perform initialization when the layer is attached to the application.
 */
 
/**
 * Perform cleanup when the layer is detached from the application.
 */

/**
 * Handle an incoming event dispatched to the layer.
 * @param e Event object received by the layer.
 */

/**
 * Render the layer's visual contents.
 */

/**
 * Update the layer's state.
 * @param dt Time elapsed since the last update in seconds.
 */
class AppLayer : public koala::core::Layer {
public:
    void OnAttach() override;
    void OnDetach() override;
    void OnEvent(koala::core::Event &e) override;
    void OnRender() override;
    void OnUpdate(float dt) override;
};
