#pragma once
#include "Window.h"


namespace core
{
    enum class EventType
    {
        WindowClose,
        //WindowResize,
        //WindowFocus,
        //WindowLostFocus,
        CursorMoved,
        KeyPressed,
        KeyReleased,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseScrolled,
    };

    class Event
    {
    public:
        EventType type;
        Window& window;
        union
        {
            struct
            {
                int width, height;
            } windowResizeEvent;

            struct
            {
                int key, scancode, mods;
            } keyEvent;

            struct
            {
                int button;
            } mouseEvent;

            struct
            {
                glm::dvec2 pos;
            } cursorEvent;
        };
    };
}
