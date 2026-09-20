#pragma once
#include "Scene.h"

class Player;
class Enemy;

class TownScene : public Scene
{
public:
    TownScene();
    virtual ~TownScene();

public:
    virtual void Start() override;
    void Update() override;
    virtual void Render() override;

private:
    shared_ptr<Shader> _shader;
    shared_ptr <Player> mPlayer;

    bool _tensionActive = false;

    float _tensionEnterRange = 15.f;
    float _tensionLeaveRange = 17.f;
};
