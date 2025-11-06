#pragma once
#include "Layer.h"

class MenuLayer final : public core::Layer
{
public:
    MenuLayer(const std::string& name = "MenuLayer");
    ~MenuLayer() override = default;
    bool onEvent(core::Event& e) override;
    void onRender() override;
    void onAttach() override;
    void onDetach() override;
};
