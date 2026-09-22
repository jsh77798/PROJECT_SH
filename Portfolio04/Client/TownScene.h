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
    void ApplyEnvironmentColors();

    void OnDoorTransitionCompleted(
        const std::wstring& triggerName,
        const std::string& defaultBGM);

    void OnCLD3Defeated();

private:
    shared_ptr<Shader> _shader;
    shared_ptr <Player> mPlayer;
    shared_ptr<Shader> _mapShader;
    shared_ptr<Shader> _skyShader;

    bool _tensionActive = false;

    float _tensionEnterRange = 15.f;
    float _tensionLeaveRange = 17.f;

    bool _cld3MusicActive = false;
    bool _cld3Defeated = false;
    bool _postCLD3MusicStarted = false;

    bool _sunsetActive = false;
};
