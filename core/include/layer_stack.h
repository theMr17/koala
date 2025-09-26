#pragma once
#include <algorithm>
#include <memory>
#include <vector>
#include "layer.h"

/**
 * Insert a regular layer at the current insert index and attach it.
 * @param layer The layer to insert into the non-overlay portion of the stack.
 */
 
/**
 * Append an overlay layer to the top of the stack and attach it.
 * @param layer The overlay layer to append.
 */
 
/**
 * Remove a regular layer from the non-overlay portion of the stack and detach it if found.
 * @param layer The layer to remove; no action is taken if it is not found among regular layers.
 */
 
/**
 * Remove an overlay layer from the overlay portion of the stack and detach it if found.
 * @param layer The overlay layer to remove; no action is taken if it is not found among overlays.
 */
 
/**
 * Return an iterator to the first layer in the stack.
 * @returns Iterator pointing to the first element.
 */

/**
 * Return an iterator one past the last layer in the stack.
 * @returns Iterator pointing one past the last element.
 */

/**
 * Return a reverse iterator to the last layer in the stack.
 * @returns Reverse iterator pointing to the last element.
 */

/**
 * Return a reverse iterator one past the first layer in reverse order.
 * @returns Reverse iterator pointing one past the first element in reverse traversal.
 */
namespace koala::core {
    class LayerStack {
    public:
        using LayerPtr = std::shared_ptr<Layer>;
        void PushLayer(const LayerPtr &layer) {
            m_Layers.emplace(m_Layers.begin() + m_InsertIndex, layer);
            m_InsertIndex++;
            layer->OnAttach();
        }
        void PushOverlay(const LayerPtr &layer) {
            m_Layers.emplace_back(layer);
            layer->OnAttach();
        }
        void PopLayer(const LayerPtr &layer) {
            auto it = std::find(m_Layers.begin(), m_Layers.begin() + (long) m_InsertIndex, layer);
            if (it != m_Layers.begin() + (long) m_InsertIndex) {
                (*it)->OnDetach();
                m_Layers.erase(it);
                m_InsertIndex--;
            }
        }
        void PopOverlay(const LayerPtr &layer) {
            auto it = std::find(m_Layers.begin() + (long) m_InsertIndex, m_Layers.end(), layer);
            if (it != m_Layers.end()) {
                (*it)->OnDetach();
                m_Layers.erase(it);
            }
        }
        auto begin() { return m_Layers.begin(); }
        auto end() { return m_Layers.end(); }
        auto rbegin() { return m_Layers.rbegin(); }
        auto rend() { return m_Layers.rend(); }

    private:
        std::vector<LayerPtr> m_Layers;
        size_t m_InsertIndex = 0; // separates regular layers and overlays};
    };
} // namespace koala::core
