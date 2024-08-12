#include "pch.hpp"
#include "Astranox/core/LayerStack.hpp"

namespace Astranox
{
    void LayerStack::pushLayer(Layer* layer)
    {
        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
        m_LayerInsertIndex++;

        layer->onAttach();
    }

    void LayerStack::pushOverlay(Layer* overlay)
    {
        m_Layers.emplace_back(overlay);
        overlay->onAttach();
    }

    void LayerStack::popLayer(Layer* layer)
    {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
        if (it != m_Layers.end())
        {
            m_Layers.erase(it);
            m_LayerInsertIndex--;
        }
        layer->onDetach();
    }

    void LayerStack::popOverlay(Layer* overlay)
    {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
        if (it != m_Layers.end())
        {
            m_Layers.erase(it);
        }
        overlay->onDetach();
    }

    void LayerStack::clear()
    {
        for (Layer* layer : m_Layers)
        {
            layer->onDetach();
            delete layer;
        }
    }
}
