#pragma once
#include "Scene.h"

class Player;

class TownScene : public Scene
{
public:
    TownScene();
    virtual ~TownScene();

public:
    virtual void Start() override;
    virtual void Update() override;
    virtual void Render() override;

private:
    shared_ptr<Shader> _shader;
    shared_ptr <Player> mPlayer;
};
