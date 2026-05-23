#pragma once

#include "Preferences.h"
#include "AudioTools.h"

class SoundVolume {
    public:
        SoundVolume(
            VolumeStream& volumeStream,
            const uint8_t minVolume,
            const uint8_t maxVolume,
            const uint8_t defaultVolume = 0
            ):  m_volumeStream(volumeStream),
                m_minVolume(minVolume),
                m_maxVolume(maxVolume),
                m_defaultVolume(defaultVolume){
        }
        void begin();
        void setCurrentVolume(uint8_t newValue);
        uint8_t getCurrentVolume();
        uint8_t getMinVolume() const;
        uint8_t getMaxVolume() const;
        uint8_t getDefaultVolume() const;
    private:
        Preferences m_preferences;
        char m_preferencesNamespace[16] = "SoundVolume";
        char m_preferencesKeyVolume[16] = "volume";
        VolumeStream& m_volumeStream;
        uint8_t m_currentVolume = 10;
        uint8_t m_minVolume;
        uint8_t m_maxVolume;
        uint8_t m_defaultVolume;
        void applyVolumeToSoundStream();
};
