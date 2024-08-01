#pragma once
#include "Layer.hpp"

namespace Astranox
{
    class LayerStack final
    {
    public:
        LayerStack() = default;
        ~LayerStack() = default;

    public:
        void pushLayer(Layer* layer);
        void pushOverlay(Layer* overlay);
        void popLayer(Layer* layer);
        void popOverlay(Layer* overlay);

        void clear();

    public: // Iterators for range-based for loop
        std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
        std::vector<Layer*>::iterator end() { return m_Layers.end(); }

        std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
        std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

    private:
        std::vector<Layer*> m_Layers;
        uint32_t m_LayerInsertIndex = 0;
    };
}
