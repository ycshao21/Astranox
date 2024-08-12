#pragma once
#include "Timestep.hpp"

namespace Astranox
{
    /**
     * This class is provided for user to create their own layers.
     */
    class Layer
    {
    public:
        Layer(const std::string& name = "Layer") : m_DebugName(name) {}
        virtual ~Layer() = default;

        virtual void onAttach() {}
        virtual void onDetach() {}
        virtual void onUpdate(Timestep ts) {}
        virtual void onEvent(Event& e) {}

        inline virtual const std::string& getName() const final { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };
}
