#pragma once
#include "Layer.h"

class DebugLayer final : public core::Layer
{
public:
    DebugLayer(const std::string& name = "DebugLayer");
    ~DebugLayer() override = default;
    void onUpdate(double dt) override;
    void onRender() override;
};
