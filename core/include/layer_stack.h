#pragma once
#include <algorithm>
#include <memory>
#include <vector>
#include "layer.h"

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
