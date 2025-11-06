#pragma once

#include <memory>
#include <vector>
#include "Layer.h"
#include "Window.h"
#include "cstmlib/Log.h"

namespace core
{
    class Application
    {
    public:
        Application(WindowSettings& settings);
        ~Application();

        template<typename T, typename ...Args> requires std::is_base_of_v<Layer, T>
        void pushLayer(Args... args);
        void suspendLayer(const std::string& name) const;
        void resumeLayer(const std::string& name) const;
        void removeLayer(const std::string& name);
        Layer* getLayer(const std::string& name);

        static Application& get();
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

    template <typename T, typename ... Args> requires std::is_base_of_v<Layer, T>
    void Application::pushLayer(Args... args)
    {
        m_Layers.emplace_back(std::make_unique<T>(args...));
        LOG_INFO("pushing layer: {}", m_Layers.back()->m_Name);
        m_Layers.back()->onAttach();
    }
}
