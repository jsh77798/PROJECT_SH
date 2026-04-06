#pragma once

class TownScene;

class GameManager
{
public:
    static GameManager& Get();

public:
    void Init();
    void Update();

private:
    shared_ptr <TownScene> Scene;
};

