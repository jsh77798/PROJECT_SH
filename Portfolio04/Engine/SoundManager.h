#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "miniaudio/miniaudio.h"

class SoundManager
{
public:
    static SoundManager& Get();

    bool Init();
    void Shutdown();

    bool LoadSFX(
        const std::string& name,
        const std::string& path,
        int voiceCount = 8);

    bool PlaySFX(
        const std::string& name,
        float volume = 1.f);

    bool PlayBGM(const std::string& path);
    void StopBGM();

    void SetMasterVolume(float volume);
    void SetBGMVolume(float volume);
    void SetSFXVolume(float volume);

    bool LoadTensionBGM(const std::string& path);

    void SetTensionActive(bool active, bool immediate = false);
    void SetTensionVolume(float volume);

    void StopTensionBGM();

    // 페이드 볼륨 갱신: 매 프레임 한 번 호출
    void Update(float dt);

private:
    SoundManager() = default;
    ~SoundManager();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    struct SoundVoice
    {
        ma_sound sound{};
        bool initialized = false;

        ~SoundVoice()
        {
            if (initialized)
                ma_sound_uninit(&sound);
        }

        SoundVoice() = default;

        SoundVoice(const SoundVoice&) = delete;
        SoundVoice& operator=(const SoundVoice&) = delete;
    };

    struct SoundPool
    {
        std::vector<std::unique_ptr<SoundVoice>> voices;
    };

private:
    ma_engine _engine{};
    ma_sound_group _sfxGroup{};

    bool _initialized = false;

    std::unordered_map<std::string, SoundPool> _sfx;
    std::unique_ptr<SoundVoice> _bgm;

    std::string _currentBGM;

    float _masterVolume = 1.f;
    float _bgmVolume = 0.4f;
    float _sfxVolume = 0.8f;

    std::unique_ptr<SoundVoice> _tensionBGM;
    std::string _currentTensionBGM;

    float _tensionVolume = 0.6f;

    // 실제 페이드 비율: 0 = 무음, 1 = 설정 볼륨
    float _tensionFade = 0.f;
    float _tensionTarget = 0.f;

    float _tensionFadeInSeconds = 2.0f;
    float _tensionFadeOutSeconds = 2.0f;
};