#include "AudioListener.h"
#include "Platform/OpenAL/ALUtilities.h"
#include "Core/Components/Camera/CameraController.h"
#include "Core/MxObject/MxObject.h"

namespace MxEngine
{
    void AudioListener::OnUpdate(float timeDelta)
    {
        auto& object = MxObject::GetByComponent(*this);
        auto position = object.Transform.GetPosition();
        auto camera = object.GetComponent<CameraController>();

        if (camera.IsValid())
        {
            Vector3 orientation[] = {
                camera->GetDirection(),
                camera->GetDirectionUp()
            };
            ALCALL(alListener3f(AL_POSITION, position.x, position.y, position.z));
            ALCALL(alListenerfv(AL_ORIENTATION, &orientation[0][0]));
        }
    }

    void AudioListener::SetVolume(float volume)
    {
        this->volume = Max(0.001f, volume);
        ALCALL(alListenerf(AL_GAIN, this->volume));
    }

    float AudioListener::GetVolume() const
    {
        return this->volume;
    }

    void AudioListener::SetVelocity(const Vector3& velocity)
    {
        this->velocity = velocity;
        ALCALL(alListener3f(AL_VELOCITY, this->velocity.x, this->velocity.y, this->velocity.z));
    }

    void AudioListener::SetSoundSpeed(float value)
    {
        this->soundSpeed = Max(0.001f, value);
        ALCALL(alSpeedOfSound(this->soundSpeed));
    }

    void AudioListener::SetDopplerFactor(float factor)
    {
        this->dopplerFactor = Max(0.0f, factor);
        ALCALL(alDopplerFactor(this->dopplerFactor));
    }

    ALenum soundModelTable[] = {
        AL_NONE,
        AL_INVERSE_DISTANCE,
        AL_INVERSE_DISTANCE_CLAMPED,
        AL_LINEAR_DISTANCE,
        AL_LINEAR_DISTANCE_CLAMPED,
        AL_EXPONENT_DISTANCE,
        AL_EXPONENT_DISTANCE_CLAMPED,
    };

    void AudioListener::SetSoundModel(SoundModel model)
    {
        ALCALL(alDistanceModel(soundModelTable[(size_t)model]));
    }

    const Vector3& AudioListener::GetVelocity() const
    {
        return this->velocity;
    }

    float AudioListener::GetSoundSpeed() const
    {
        return this->soundSpeed;
    }

    float AudioListener::GetDopplerFactor() const
    {
        return this->dopplerFactor;
    }
}