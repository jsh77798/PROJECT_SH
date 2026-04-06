#pragma once
#include "IExecute.h"

class GameManager;

class AppManager : public IExecute
{
public:
    AppManager();
    virtual ~AppManager();

public:
    virtual void Init() override;
    virtual void Update() override;
    virtual void Render() override;

private:
    shared_ptr <GameManager> game;
    shared_ptr<Shader> _shader;
};
