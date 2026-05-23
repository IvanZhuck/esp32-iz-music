#define USE_AUDIO_LOGGING 0

#include <Arduino.h>
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include "SoundVolume.h"
#include "Display.h"
#include "Encoder.h"
#include "SPIFFS.h"
#include "AudioTools/AudioLibs/AudioRealFFT.h"

constexpr uint8_t I2S_PIN_BCK = 27;
constexpr uint8_t I2S_PIN_WS = 26;
constexpr uint8_t I2S_PIN_DATA = 25;

constexpr uint8_t VOLUME_MIN = 0;
constexpr uint8_t VOLUME_MAX = 100;
constexpr uint8_t VOLUME_DEFAULT = 10;

constexpr uint16_t FFT_LENGTH = 512;

constexpr int MIXER_BUFFER_MAX_SIZE = 5000;

I2SStream i2sStream;

VolumeStream volumeStream(i2sStream);
Equalizer3Bands equalizerStream(volumeStream);
EncodedAudioStream decoder(&i2sStream, new WAVDecoder());
AudioRealFFT fft;

BluetoothA2DPSink a2dpSink;
SoundVolume soundVolume(volumeStream, VOLUME_MIN, VOLUME_MAX, VOLUME_DEFAULT);
Display display(a2dpSink, soundVolume, fft);
Encoder encoder(a2dpSink, display, soundVolume);

SineWaveGenerator<int16_t> sineWave(500);
GeneratedSoundStream<int16_t> generatedSineSound(sineWave);
OutputMixer<int16_t> outputMixer(equalizerStream, 2);

uint8_t mixerBuffer[MIXER_BUFFER_MAX_SIZE];

void playLocalFile(const String& filePath) {
    File file = SPIFFS.open(filePath, "r");
    decoder.begin();
    StreamCopy copierLocalPlay(decoder, file);
    copierLocalPlay.copyAll();
    file.close();
    decoder.end();
}

void a2dpDataCallback(const uint8_t* data, uint32_t length) {
    generatedSineSound.readBytes(mixerBuffer, length);

    outputMixer.write(mixerBuffer, length);
    outputMixer.write(data, length);
}

void a2dpRawDataCallback(const uint8_t* data, uint32_t length) {
    fft.write(data, length);
}

void a2dpConnectionStateChanged(esp_a2d_connection_state_t state, void *ptr) {
    if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
        playLocalFile("/connected.wav");
    }
}

void a2dpAudioStateChanged(esp_a2d_audio_state_t state, void *ptr){
    if (state == ESP_A2D_AUDIO_STATE_STARTED) {
        a2dpSink.set_stream_reader(a2dpDataCallback, false);
    } else {
        a2dpSink.set_stream_reader(nullptr, false);
    }
}

void setup() {
    Serial.begin(115200);
    SPIFFS.begin();

    auto i2sConfig = i2sStream.defaultConfig();
    i2sConfig.sample_rate = 44100;
    i2sConfig.buffer_count = 6;
    i2sConfig.buffer_size = 512;
    i2sConfig.pin_bck = I2S_PIN_BCK;
    i2sConfig.pin_ws = I2S_PIN_WS;
    i2sConfig.pin_data = I2S_PIN_DATA;
    i2sStream.begin(i2sConfig);

    auto equalizerConfig = equalizerStream.defaultConfig();
    equalizerConfig.copyFrom(i2sConfig);
    equalizerConfig.gain_low = 1.8;
    equalizerConfig.gain_medium = 0.7;
    equalizerConfig.gain_high = 0.5;
    equalizerStream.begin(equalizerConfig);

    auto volumeConfig = volumeStream.defaultConfig();
    volumeConfig.copyFrom(i2sConfig);
    volumeStream.begin(volumeConfig);

    outputMixer.begin(MIXER_BUFFER_MAX_SIZE);
    auto sineWaveConfig = sineWave.defaultConfig();
    sineWaveConfig.copyFrom(i2sConfig);
    sineWave.begin(sineWaveConfig, 3.0f);

    auto fftConfig = fft.defaultConfig();
    fftConfig.copyFrom(i2sConfig);
    fftConfig.length = FFT_LENGTH;
    fftConfig.window_function = new BufferedWindow(new Hann());
    fft.begin(fftConfig);

    soundVolume.begin();
    display.begin();
    encoder.begin();

    display.renderLoadingScreen();
    playLocalFile("/power_on.wav");

    a2dpSink.set_auto_reconnect(true, 1000);
    a2dpSink.set_stream_reader(a2dpDataCallback, false);
    a2dpSink.set_raw_stream_reader(a2dpRawDataCallback);
    a2dpSink.set_on_connection_state_changed(a2dpConnectionStateChanged);
    a2dpSink.set_on_audio_state_changed(a2dpAudioStateChanged);
    a2dpSink.start("IZMusic");
}

void loop() {
    display.tick();
}