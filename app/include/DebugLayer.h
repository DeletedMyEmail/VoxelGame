#pragma once
#include "GameLayer.h"
#include "Layer.h"

class DebugLayer final : public core::Layer
{
public:
    DebugLayer(const std::string& name = "DebugLayer");
    ~DebugLayer() override = default;
    void onUpdate(double dt) override;
    void onRender() override;
    bool onEvent(core::Event& e) override;
private:
    void drawMenu() const;
    void drawAxes();
private:
    GameLayer* m_GameLayer = nullptr;
    VertexArray m_AxisVao;
    Shader m_Shader;
};
