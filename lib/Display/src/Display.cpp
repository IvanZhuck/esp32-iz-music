#include "Display.h"
#include "bitmaps.h"

void Display::begin() {
    Wire.begin(DISPLAY_PIN_SDA, DISPLAY_PIN_SLC);
    m_display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    m_display.setTextColor(WHITE);

    clearScreen();
    drawScreenFromFrameBuffer();
}

void Display::tick() {
    m_currentExecutedTime = millis();

    if (m_backToMainScreenInMillis > 0) {
        unsigned long deltaExecutedTime = m_currentExecutedTime - m_prevExecutedTime;

        if (m_backToMainScreenInMillis > deltaExecutedTime) {
            m_backToMainScreenInMillis -= deltaExecutedTime;
        } else {
            m_backToMainScreenInMillis = 0;
        }
    }

    if (m_backToMainScreenInMillis == 0 && m_currentExecutedTime - m_screenPrevUpdatedTime > 1000 / DISPLAY_FPS_LIMIT) {
        fillVisualizerDataRawBuffer();
        computeVisualizerData();
        renderMainScreen();
        m_screenPrevUpdatedTime = m_currentExecutedTime;
    }

    m_prevExecutedTime = m_currentExecutedTime;
}

void Display::renderLoadingScreen() {
    clearScreen();
    m_frameBuffer.setTextSize(1);
    m_frameBuffer.drawBitmap(0, 0, bitmapLogo, DISPLAY_WIDTH, DISPLAY_HEIGHT, WHITE, 0);
    drawScreenFromFrameBuffer();
}

void Display::renderMainScreen() {
    clearScreen();
    drawVisualizer();
    drawStatusBar();
    drawScreenFromFrameBuffer();
}

void Display::renderSoundVolumeScreen() {
    clearScreen();
    drawSoundVolumeValue();
    drawScreenFromFrameBuffer();
}

void Display::drawStatusBar() {
    const String connectionStatusText = m_a2dpSink.is_connected() ? "Connected" : "Disconnected";
    constexpr uint8_t textSize = 1;

    int16_t currentCursorX, currentCursorY;
    uint16_t textWidth, textHeight;

    m_display.getTextBounds(
        connectionStatusText, 0, 0,
        &currentCursorX, &currentCursorY,
        &textWidth, &textHeight
    );

    int16_t cursorX = DISPLAY_WIDTH - textWidth * textSize;
    int16_t cursorY = DISPLAY_HEIGHT - textHeight * textSize;

    m_frameBuffer.setTextSize(textSize);
    m_frameBuffer.setCursor(cursorX, cursorY);
    m_frameBuffer.print(connectionStatusText);
    m_frameBuffer.drawBitmap(0, 56, bitmapIconSoundVolume, 10, 7, WHITE, 0);
    m_frameBuffer.setCursor(14, 56);
    m_frameBuffer.print(m_soundVolume.getCurrentVolume());
}

void Display::drawVisualizer() {
    constexpr uint8_t marginFromDisplayTop = DISPLAY_VISUALIZER_ROWS + 4;
    constexpr uint8_t columnWidth = DISPLAY_WIDTH / DISPLAY_VISUALIZER_COLS;
    constexpr uint8_t spaceBetweenCols = 2;
    constexpr uint8_t emptyColHeight = 2;

    for (uint8_t i = 0; i < DISPLAY_VISUALIZER_COLS; i++) {
        uint8_t colHeight = m_visualizerData[i];

        m_frameBuffer.fillRect(
            i * columnWidth + 1,
            std::min<uint8_t>(marginFromDisplayTop - colHeight, marginFromDisplayTop - emptyColHeight),
            columnWidth - spaceBetweenCols,
            std::max<uint8_t>(colHeight, 2),
            WHITE
        );
    }
}

void Display::drawSoundVolumeValue() {
    m_backToMainScreenInMillis = 2000;
    uint8_t textSize = 5;

    char textBufferVolumeValue[5] = "MUTE";

    if (m_soundVolume.getCurrentVolume() > 0) {
        sprintf(textBufferVolumeValue, "%d", m_soundVolume.getCurrentVolume());
    }

    int16_t currentCursorX, currentCursorY;
    uint16_t textWidth, textHeight;

    m_frameBuffer.setTextSize(textSize);

    m_display.getTextBounds(
        textBufferVolumeValue, 0, 0,
        &currentCursorX, &currentCursorY,
        &textWidth, &textHeight
    );

    int16_t cursorX = (DISPLAY_WIDTH - textWidth * textSize) / 2;
    int16_t cursorY = (DISPLAY_HEIGHT - textHeight * textSize) / 2;

    m_frameBuffer.setCursor(cursorX, cursorY);
    m_frameBuffer.print(textBufferVolumeValue);
}

void Display::fillVisualizerDataRawBuffer() {
    float* magnitudes = m_fft.magnitudes();
    uint8_t percent = 0;

    for (int i = 0; i < m_fft.config().length / 2; i++) {
        /**
         * Значения амплитуд изменяются от 0 до 3-6 в зависимости от громкости звука.
         * Но бывают выпады и выше. Коэффециент 0.05 подобран "на глаз". Чтобы примерно загнать
         * амплитуду в процентный диапазон от 0 до 100.
         */
        percent = std::min<uint8_t>(static_cast<uint8_t>(magnitudes[i] / 0.05), 100);
        m_visualizerDataRawBuffer[i] = static_cast<uint8_t>(DISPLAY_VISUALIZER_ROWS * 0.01 * percent);
    }
}

void Display::computeVisualizerData() {
    for (uint8_t i = 0; i < DISPLAY_VISUALIZER_COLS; i++) {
        unsigned int totalValue = 0;
        uint8_t length = 0;

        for (uint16_t j = m_visualizerDataRanges[i][0]; j < m_visualizerDataRanges[i][1] + 1; j++) {
            if (m_visualizerDataRawBuffer[j] < 10) {
                continue;
            }

            totalValue += m_visualizerDataRawBuffer[j];
            length++;
        }

        length = std::max<uint8_t>(length, 1);

        /*
         * Для визуализации используется среднее от суммы текущего значения и предыдущего.
         * Это нужно для плавной анимации колонок визуализатора.
         */
        m_visualizerData[i] = static_cast<uint8_t>((totalValue / length + m_visualizerData[i]) / 2);
    }
}

void Display::drawScreenFromFrameBuffer() {
    m_display.clearDisplay();
    m_display.drawBitmap(0, 0, m_frameBuffer.getBuffer(), DISPLAY_WIDTH, DISPLAY_HEIGHT, 1, 0);
    m_display.display();
}

void Display::clearScreen() {
    m_frameBuffer.fillScreen(0);
}