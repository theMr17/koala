#include "app_layer.h"


/**
 * @brief Invoked when the layer is attached to the application.
 *
 * Intended for initialization that should occur when the layer becomes active.
 * The default implementation performs no action.
 */
void AppLayer::OnAttach() {}

/**
 * @brief Called when the layer is detached from the application.
 *
 * Default no-op implementation; override to perform teardown or cleanup when the layer is detached.
 */
void AppLayer::OnDetach() {}

/**
 * @brief Receives and processes an incoming event for this layer.
 *
 * Default no-op implementation; override in derived classes to handle or propagate the event.
 *
 * @param e The event to be handled by the layer.
 */
void AppLayer::OnEvent(koala::core::Event &e) {}

/**
 * @brief Called once per frame to update the layer.
 *
 * Receives the elapsed time since the previous update and allows the layer to advance its state.
 * The default implementation performs no action.
 *
 * @param dt Time elapsed since the last update, in seconds.
 */
void AppLayer::OnUpdate(float dt) {}

/**
 * @brief Renders this layer's visual contents.
 *
 * Default no-op implementation; override in derived classes to perform actual rendering.
 */
void AppLayer::OnRender() {}
