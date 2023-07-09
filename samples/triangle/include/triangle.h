#pragma once
#include "game_runtime/GameApplication.h"

class Triangle : public Cyber::GameApplication
{
public:
    Triangle();
    ~Triangle();

    virtual void initialize(Cyber::WindowDesc& desc) override;
    virtual void Run() override;
    virtual void onEvent(Cyber::Event& e) override;
};

