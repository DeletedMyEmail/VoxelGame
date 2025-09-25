#pragma once

#include <vector>
#include "Layer.h"
#include "Window.h"

namespace core
{
    class Application
    {
    public:
        Application(WindowSettings& settings);
        ~Application();

        static Application& get();
        template<typename T> requires std::is_base_of_v<Layer, T>
        void pushLayer() { m_Layers.emplace_back(new T()); m_Layers.back()->onAttach(); }
        void run();
        void stop();
        void propagateEvent(Event& e) const;
        // Returns time in seconds
        double getTime() const;
        Window& getWindow() { return m_Window; }
    private:
        std::vector<std::unique_ptr<Layer>> m_Layers;
        Window m_Window;
        bool m_Running = false;
    };
}
