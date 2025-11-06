#pragma once
#include "Layer.h"

class ControlLayer final : public core::Layer
{
public:
    ControlLayer(const std::string& name = "ControlLayer") : Layer(name) {}
    ~ControlLayer() override = default;
    bool onEvent(core::Event& e) override;
};
