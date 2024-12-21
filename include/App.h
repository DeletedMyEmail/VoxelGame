#pragma once

#include "Renderer.h"

class App {
public:
    App();
    ~App() = default;

    void run();
private:
  Renderer m_Renderer;
};