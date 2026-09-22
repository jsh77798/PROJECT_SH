#include "pch.h"
#include "SoundManager.h"

#include <cstdio>

namespace
{
    float ClampAudioVolume(float value)
    {
        if (value < 0.f)
            return 0.f;

        if (value > 1.f)
            return 1.f;

        return value;
    }

    bool CheckAudio(
        ma_result result,
        const char* operation,
        const std::string& detail = "")
    {
        if (result == MA_SUCCESS)
            return true;

        char buffer[1024];

        sprintf_s(
            buffer,
            "[Audio] %s failed: result=%d, %s\n",
            operation,
            static_cast<int>(result),
            detail.c_str()
        );

        OutputDebugStringA(buffer);
        return false;
    }
}

SoundManager& SoundManager::Get()
{
    static SoundManager instance;
    return instance;
}

SoundManager::~SoundManager()
{
    Shutdown();
}

bool SoundManager::Init()
{
    if (_initialized)
        return true;

    ma_engine_config config = ma_engine_config_init();

    if (!CheckAudio(
        ma_engine_init(&config, &_engine),
        "Engine init"))
    {
        return false;
    }

    // 효과음 전체 볼륨을 제어할 그룹
    if (!CheckAudio(
        ma_sound_group_init(
            &_engine,
            0,
            nullptr,
            &_sfxGroup),
        "SFX group init"))
    {
        ma_engine_uninit(&_engine);
        return false;
    }

    _initialized = true;

    SetMasterVolume(_masterVolume);
    SetSFXVolume(_sfxVolume);

    return true;
}

void SoundManager::Shutdown()
{
    if (!_initialized)
        return;

    // 사운드 -> 그룹 -> 엔진 순서로 해제
    StopTensionBGM();
    StopBGM();
    _sfx.clear();

    ma_sound_group_uninit(&_sfxGroup);
    ma_engine_uninit(&_engine);

    _initialized = false;
}

bool SoundManager::LoadSFX(
    const std::string& name,
    const std::string& path,
    int voiceCount)
{
    if (!_initialized)
        return false;

    if (voiceCount <= 0)
        return false;

    // 같은 이름으로 이미 로드되어 있으면 재사용
    if (_sfx.find(name) != _sfx.end())
        return true;

    SoundPool pool;

    for (int i = 0; i < voiceCount; ++i)
    {
        auto voice = std::make_unique<SoundVoice>();

        ma_result result;

        if (i == 0)
        {
            // 짧은 효과음은 미리 디코딩
            result = ma_sound_init_from_file(
                &_engine,
                path.c_str(),
                MA_SOUND_FLAG_DECODE |
                MA_SOUND_FLAG_NO_SPATIALIZATION,
                &_sfxGroup,
                nullptr,
                &voice->sound
            );
        }
        else
        {
            // 동일 데이터를 사용하는 추가 재생 음성
            result = ma_sound_init_copy(
                &_engine,
                &pool.voices.front()->sound,
                MA_SOUND_FLAG_NO_SPATIALIZATION,
                &_sfxGroup,
                &voice->sound
            );
        }

        if (!CheckAudio(result, "Load SFX", path))
            return false;

        voice->initialized = true;

        ma_sound_set_looping(
            &voice->sound,
            MA_FALSE
        );

        pool.voices.push_back(std::move(voice));
    }

    _sfx.emplace(name, std::move(pool));
    return true;
}

bool SoundManager::PlaySFX(
    const std::string& name,
    float volume)
{
    if (!_initialized)
    {
        OutputDebugStringA(
            "[Audio] PlaySFX: engine not initialized\n"
        );

        return false;
    }

    auto it = _sfx.find(name);

    if (it == _sfx.end())
    {
        OutputDebugStringA(
            ("[Audio] SFX not loaded: " + name + "\n").c_str()
        );
        return false;
    }

    // 사용하지 않는 음성을 찾아 재생
    for (auto& voice : it->second.voices)
    {
        ma_sound* sound = &voice->sound;

        if (ma_sound_is_playing(sound))
            continue;

        if (!CheckAudio(
            ma_sound_seek_to_pcm_frame(sound, 0),
            "Rewind SFX",
            name))
        {
            return false;
        }

        ma_sound_set_volume(
            sound,
            ClampAudioVolume(volume)
        );

        return CheckAudio(
            ma_sound_start(sound),
            "Play SFX",
            name
        );
    }

    OutputDebugStringA(
        ("[Audio] PlaySFX: all voices busy: "
            + name + "\n").c_str()
    );

    // 모두 재생 중이면 이번 소리만 생략
    return false;
}

bool SoundManager::PlayBGM(const std::string& path)
{
    if (!_initialized)
        return false;

    // 같은 곡을 반복 요청해도 처음부터 재시작하지 않음
    if (_bgm && _currentBGM == path)
        return true;

    auto next = std::make_unique<SoundVoice>();

    if (!CheckAudio(
        ma_sound_init_from_file(
            &_engine,
            path.c_str(),
            MA_SOUND_FLAG_STREAM |
            MA_SOUND_FLAG_NO_SPATIALIZATION,
            nullptr,
            nullptr,
            &next->sound),
        "Load BGM",
        path))
    {
        // 새 곡을 못 읽으면 기존 곡 유지
        return false;
    }

    next->initialized = true;

    ma_sound_set_looping(&next->sound, MA_TRUE);
    ma_sound_set_volume(&next->sound, _bgmVolume);

    if (!CheckAudio(
        ma_sound_start(&next->sound),
        "Play BGM",
        path))
    {
        return false;
    }

    StopBGM();

    _bgm = std::move(next);
    _currentBGM = path;

    return true;
}

void SoundManager::StopBGM()
{
    _bgm.reset();
    _currentBGM.clear();
}

void SoundManager::SetMasterVolume(float volume)
{
    _masterVolume = ClampAudioVolume(volume);

    if (_initialized)
        ma_engine_set_volume(&_engine, _masterVolume);
}

void SoundManager::SetBGMVolume(float volume)
{
    _bgmVolume = ClampAudioVolume(volume);

    if (_bgm)
        ma_sound_set_volume(&_bgm->sound, _bgmVolume);
}

void SoundManager::SetSFXVolume(float volume)
{
    _sfxVolume = ClampAudioVolume(volume);

    if (_initialized)
        ma_sound_group_set_volume(&_sfxGroup, _sfxVolume);
}

bool SoundManager::LoadTensionBGM(const std::string& path)
{
    if (!_initialized)
        return false;

    if (_tensionBGM && _currentTensionBGM == path)
        return true;

    auto next = std::make_unique<SoundVoice>();

    if (!CheckAudio(
        ma_sound_init_from_file(
            &_engine,
            path.c_str(),
            MA_SOUND_FLAG_STREAM |
            MA_SOUND_FLAG_NO_SPATIALIZATION,
            nullptr,
            nullptr,
            &next->sound),
        "Load tension BGM",
        path))
    {
        return false;
    }

    next->initialized = true;

    ma_sound_set_looping(
        &next->sound,
        MA_TRUE
    );

    // 처음에는 무음으로 시작
    ma_sound_set_volume(
        &next->sound,
        0.f
    );

    if (!CheckAudio(
        ma_sound_start(&next->sound),
        "Start tension BGM",
        path))
    {
        return false;
    }

    StopTensionBGM();

    _tensionBGM = std::move(next);
    _currentTensionBGM = path;

    return true;
}

void SoundManager::SetTensionActive(bool active, bool immediate)
{
    _tensionTarget = active ? 1.f : 0.f;

    if (immediate)
    {
        _tensionFade = _tensionTarget;

        if (_tensionBGM)
        {
            ma_sound_set_volume(
                &_tensionBGM->sound,
                _tensionVolume * _tensionFade
            );
        }
    }
}

void SoundManager::SetTensionVolume(float volume)
{
    _tensionVolume = ClampAudioVolume(volume);

    if (_tensionBGM)
    {
        ma_sound_set_volume(
            &_tensionBGM->sound,
            _tensionVolume * _tensionFade
        );
    }
}

void SoundManager::StopTensionBGM()
{
    _tensionBGM.reset();
    _currentTensionBGM.clear();

    _tensionFade = 0.f;
    _tensionTarget = 0.f;
}

void SoundManager::Update(float dt)
{
    if (!_initialized || !_tensionBGM || dt <= 0.f)
        return;

    if (_tensionFade < _tensionTarget)
    {
        // 페이드 인
        if (_tensionFadeInSeconds <= 0.f)
        {
            _tensionFade = _tensionTarget;
        }
        else
        {
            _tensionFade += dt / _tensionFadeInSeconds;

            if (_tensionFade > _tensionTarget)
                _tensionFade = _tensionTarget;
        }
    }
    else if (_tensionFade > _tensionTarget)
    {
        // 페이드 아웃
        if (_tensionFadeOutSeconds <= 0.f)
        {
            _tensionFade = _tensionTarget;
        }
        else
        {
            _tensionFade -= dt / _tensionFadeOutSeconds;

            if (_tensionFade < _tensionTarget)
                _tensionFade = _tensionTarget;
        }
    }

    ma_sound_set_volume(
        &_tensionBGM->sound,
        _tensionVolume * _tensionFade
    );
}
