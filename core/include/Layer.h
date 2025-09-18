#pragma once
#include "Event.h"

namespace core
{
    class Layer
    {
    public:
        virtual ~Layer() = default;
        virtual void onUpdate(float dt) {}
        virtual void onRender() {};
        virtual void onEvent(Event& e) {}
        virtual void onAttach() {}
        virtual void onDetach() {}
    };
}
