#include "ControlLayer.h"
#include "Application.h"
#include "MenuLayer.h"

bool ControlLayer::onEvent(core::Event& e)
{
    if (e.type != core::EventType::KeyPressed) return false;

    if (e.keyEvent.key == GLFW_KEY_F3)
    {
        if (!core::Application::get().getLayer("MenuLayer"))
            core::Application::get().toggleLayer("DebugLayer");
        return true;
    }
    if (e.keyEvent.key == GLFW_KEY_ESCAPE)
    {
        if (core::Application::get().getLayer("MenuLayer"))
        {
            core::Application::get().removeLayer("MenuLayer");
            core::Application::get().resumeLayer("GameLayer");
        }
        else
        {
            core::Application::get().suspendLayer("DebugLayer");
            core::Application::get().suspendLayer("GameLayer");
            core::Application::get().pushLayer<MenuLayer>("MenuLayer");
        }
        return true;
    }

    return false;
}
