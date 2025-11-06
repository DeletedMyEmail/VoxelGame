#pragma once

#include "Event.h"
#include <string>

namespace core
{
    class Layer
    {
    public:
        Layer(const std::string& name) : m_Name(name) {}
        virtual ~Layer() = default;
        virtual void onUpdate(double dt) {}
        virtual void onRender() {}
        virtual bool onEvent(Event& e) {}
        virtual void onAttach() {}
        virtual void onDetach() {}
    public:
        bool m_Enabled = true;
        const std::string m_Name;
    };
}
