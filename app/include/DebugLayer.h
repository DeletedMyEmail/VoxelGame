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
    void drawMenu(GameLayer* gameLayer) const;
    void drawAxes(GameLayer* gameLayer);
private:
    VertexArray m_AxisVao;
    Shader m_Shader;
};
