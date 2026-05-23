#pragma once

#include <ESP32RotaryEncoder.h>
#include "BluetoothA2DPSink.h"
#include "Display.h"
#include "SoundVolume.h"

constexpr uint8_t INPUT_PIN_ENCODER_LEFT = 18;
constexpr uint8_t INPUT_PIN_ENCODER_RIGHT = 19;
constexpr uint8_t INPUT_PIN_ENCODER_BUTTON = 21;

class Encoder {
    public:
        Encoder(
            BluetoothA2DPSink& a2dpSink,
            Display& display,
            SoundVolume& soundVolume
            ) : m_a2dpSink(a2dpSink),
                m_display(display),
                m_soundVolume(soundVolume),
                m_rotaryEncoder(INPUT_PIN_ENCODER_LEFT, INPUT_PIN_ENCODER_RIGHT, INPUT_PIN_ENCODER_BUTTON) {
        }

        static Encoder* s_instance;
        void begin();
        static void onTurnedCallback(long newValue);
        static void onPressedCallback(unsigned long duration);
    private:
        BluetoothA2DPSink& m_a2dpSink;
        Display& m_display;
        SoundVolume& m_soundVolume;
        RotaryEncoder m_rotaryEncoder;
};
