#include "Encoder.h"

/**
 * Статический экземпляр нужен для доступа к членам класса Encoder
 * внутри функций обратного вызова, передаваемых в RotaryEncoder
 */
Encoder* Encoder::s_instance = nullptr;

void Encoder::begin() {
    s_instance = this;
    m_rotaryEncoder.setEncoderType(EncoderType::HAS_PULLUP);
    m_rotaryEncoder.setBoundaries(m_soundVolume.getMinVolume(), m_soundVolume.getMaxVolume(), false);
    m_rotaryEncoder.onTurned(&onTurnedCallback);
    m_rotaryEncoder.onPressed(&onPressedCallback);
    m_rotaryEncoder.begin();
    m_rotaryEncoder.setEncoderValue(m_soundVolume.getCurrentVolume());
}

void Encoder::onTurnedCallback(long newValue) {
    if (!s_instance) {
        throw std::runtime_error("No Encoder instance");
    }

    s_instance->m_soundVolume.setCurrentVolume(static_cast<uint8_t>(newValue));
    s_instance->m_display.renderSoundVolumeScreen();
}

void Encoder::onPressedCallback(unsigned long duration) {
    if (!s_instance) {
        throw std::runtime_error("No Encoder instance");
    }

    // Короткое нажатие - пауза/воспроизведение
    if (duration < 1000) {
        if (s_instance->m_a2dpSink.get_audio_state() == ESP_A2D_AUDIO_STATE_STARTED) {
            s_instance->m_a2dpSink.pause();
        } else {
            s_instance->m_a2dpSink.play();
        }
    }

    // Длинное нажатие - разрыв соединения
    if (duration > 5000) {
        s_instance->m_a2dpSink.pause();
        delay(500);
        s_instance->m_a2dpSink.disconnect();
    }
}
