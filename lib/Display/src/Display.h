#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothA2DPSink.h"
#include "SoundVolume.h"
#include "AudioTools/AudioLibs/AudioRealFFT.h"

constexpr uint8_t DISPLAY_PIN_SDA = 16;
constexpr uint8_t DISPLAY_PIN_SLC = 17;
constexpr uint8_t DISPLAY_WIDTH = 128;
constexpr uint8_t DISPLAY_HEIGHT = 64;
constexpr uint8_t DISPLAY_FPS_LIMIT = 30;

constexpr uint8_t DISPLAY_VISUALIZER_COLS = 16;
constexpr uint8_t DISPLAY_VISUALIZER_ROWS = 40;
constexpr uint16_t DISPLAY_VISUALIZER_BUFFER_SIZE = 256;

class Display {
    public:
        Display(
            BluetoothA2DPSink& a2dpSink,
            SoundVolume& soundVolume,
            AudioRealFFT& fft
            ) : m_a2dpSink(a2dpSink),
                m_soundVolume(soundVolume),
                m_fft(fft),
                m_display(DISPLAY_WIDTH, DISPLAY_HEIGHT),
                m_frameBuffer(DISPLAY_WIDTH, DISPLAY_HEIGHT) {
            m_screenPrevUpdatedTime = millis();
        }

        void begin();
        void tick();
        void renderLoadingScreen();
        void renderMainScreen();
        void renderSoundVolumeScreen();
    private:
        BluetoothA2DPSink& m_a2dpSink;
        SoundVolume& m_soundVolume;
        AudioRealFFT& m_fft;
        Adafruit_SSD1306 m_display;
        GFXcanvas1 m_frameBuffer;

        unsigned long m_screenPrevUpdatedTime = 0;
        unsigned long m_currentExecutedTime = 0;
        unsigned long m_prevExecutedTime = 0;

        uint16_t m_backToMainScreenInMillis = 0;

        uint8_t m_visualizerDataRawBuffer[DISPLAY_VISUALIZER_BUFFER_SIZE] = {0};
        uint8_t m_visualizerData[DISPLAY_VISUALIZER_COLS] = {0};

        /// Разделение спектра частот на 16 полос для далтнейшей визуализации
        uint8_t m_visualizerDataRanges[DISPLAY_VISUALIZER_COLS][2] = {
            {0,0}, //до 86,13 Гц
            {0,1}, //до 172,27 Гц
            {1,2}, //до 258,4 Гц
            {2,3}, //до 344,53 Гц
            {3,5}, //до 516,8 Гц
            {5,7}, //до 689,06 Гц
            {7,9}, //до 861,33 Гц
            {9,11}, //до 1033,59 Гц
            {11,17}, //до 1550,39 Гц
            {17,23}, //до 2067,19 Гц
            {23,31}, //до 2756,25 Гц
            {31,45}, //до 3962,1 Гц
            {45,64}, //до 5598,63 Гц
            {64,88}, //до 7665,82 Гц
            {88,120}, //до 10422,07 Гц
            {120,255} //до 22050 Гц
        };

        void drawVisualizer();
        void drawStatusBar();
        void drawSoundVolumeValue();
        void fillVisualizerDataRawBuffer();
        void computeVisualizerData();
        void drawScreenFromFrameBuffer();
        void clearScreen();
};
