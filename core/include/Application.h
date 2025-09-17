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
        void pushLayer() { m_Layers.emplace_back(T()); }
        void run();
        void stop();
        double getTime() const;
        Window& getWindow() { return m_Window; }
    private:
        std::vector<Layer> m_Layers;
        Window m_Window;
        bool m_Running = false;
    };
}
