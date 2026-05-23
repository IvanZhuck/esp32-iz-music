#include "SoundVolume.h"

void SoundVolume::begin() {
    m_preferences.begin(m_preferencesNamespace, false);
    applyVolumeToSoundStream();
}

void SoundVolume::setCurrentVolume(uint8_t newValue) {
    if (newValue < m_minVolume) {
        newValue = m_minVolume;
    }

    if (newValue > m_maxVolume) {
        newValue = m_maxVolume;
    }

    m_preferences.putUChar(m_preferencesKeyVolume, newValue);
    applyVolumeToSoundStream();
}

void SoundVolume::applyVolumeToSoundStream() {
    m_volumeStream.setVolume(getCurrentVolume() * 0.01f);
}

uint8_t SoundVolume::getCurrentVolume() {
    return m_preferences.getUChar(m_preferencesKeyVolume, m_defaultVolume);
}

uint8_t SoundVolume::getMinVolume() const {
    return m_minVolume;
}

uint8_t SoundVolume::getMaxVolume() const {
    return m_maxVolume;
}

uint8_t SoundVolume::getDefaultVolume() const {
    return m_defaultVolume;
}