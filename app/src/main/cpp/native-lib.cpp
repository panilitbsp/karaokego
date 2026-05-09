#include <jni.h>
#include <oboe/Oboe.h>
#include <android/log.h>
#include <memory>

// Import SDK Superpowered
#include <Superpowered.h>
#include <SuperpoweredReverb.h>
#include <SuperpoweredEcho.h>
#include <SuperpoweredFlanger.h>

#define LOG_TAG "MesinKaraoke"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class KaraokeEngine : public oboe::AudioStreamDataCallback {
private:
    std::shared_ptr<oboe::AudioStream> mRecordStream;
    std::shared_ptr<oboe::AudioStream> mPlayStream;

    int currentEffectMode = 0;

    // Wadah efek
    Superpowered::Reverb* reverbSyahrini;
    Superpowered::Echo* echoKaraoke;
    Superpowered::Flanger* efekTreasure;

public:
    bool start() {
        // 1. Inisialisasi Superpowered (HANYA 1 ARGUMEN)
        Superpowered::Initialize("ExampleLicenseKey-WillWorkForAWhile");

        // --- Konfigurasi Mic (Input Mono) ---
        oboe::AudioStreamBuilder recordBuilder;
        recordBuilder.setDirection(oboe::Direction::Input);
        recordBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        recordBuilder.setFormat(oboe::AudioFormat::Float);
        recordBuilder.setChannelCount(1);

        if (recordBuilder.openStream(mRecordStream) != oboe::Result::OK) {
            LOGE("Gagal buka mic!");
            return false;
        }

        // --- Konfigurasi Speaker (Output Stereo) ---
        oboe::AudioStreamBuilder playBuilder;
        playBuilder.setDirection(oboe::Direction::Output);
        playBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        playBuilder.setFormat(oboe::AudioFormat::Float);
        playBuilder.setChannelCount(2);
        playBuilder.setDataCallback(this);

        if (playBuilder.openStream(mPlayStream) != oboe::Result::OK) {
            LOGE("Gagal buka speaker!");
            return false;
        }

        int sampleRate = mPlayStream->getSampleRate();

        // 2. Setup Efek Syahrini (Menggunakan variabel 'mix')
        reverbSyahrini = new Superpowered::Reverb(sampleRate);
        reverbSyahrini->enabled = true;
        reverbSyahrini->mix = 0.3f;

        // 3. Setup Efek Karaoke (Menggunakan fungsi 'setMix')
        echoKaraoke = new Superpowered::Echo(sampleRate);
        echoKaraoke->enabled = true;
        echoKaraoke->setMix(0.4f);

        // 4. Setup Efek Rapper TREASURE (Flanger)
        efekTreasure = new Superpowered::Flanger(sampleRate);
        efekTreasure->enabled = true;

        mRecordStream->requestStart();
        mPlayStream->requestStart();
        LOGI("Mesin Superpowered & Oboe Berjalan!");
        return true;
    }

    void stop() {
        if (mPlayStream) { mPlayStream->requestStop(); mPlayStream->close(); mPlayStream.reset(); }
        if (mRecordStream) { mRecordStream->requestStop(); mRecordStream->close(); mRecordStream.reset(); }

        // Bersihkan memori saat mic dimatikan
        delete reverbSyahrini;
        delete echoKaraoke;
        delete efekTreasure;
        LOGI("Mesin Dimatikan.");
    }

    void setMode(int mode) {
        currentEffectMode = mode;
        LOGI("Mode efek diubah ke angka: %d", currentEffectMode);
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        float* outputBuffer = static_cast<float*>(audioData);
        memset(outputBuffer, 0, numFrames * 2 * sizeof(float));

        if (mRecordStream) {
            float monoBuffer[numFrames];
            mRecordStream->read(monoBuffer, numFrames, 0);

            // Ubah Mono (Mic) ke Stereo (Kiri-Kanan)
            for(int i = 0; i < numFrames; i++) {
                outputBuffer[i*2]     = monoBuffer[i];
                outputBuffer[i*2 + 1] = monoBuffer[i];
            }

            // PABRIK MAKE-UP SUARA BERDASARKAN PILIHAN UI
            if (currentEffectMode == 1) {
                // Mode 1: Syahrini (Reverb elegan)
                reverbSyahrini->process(outputBuffer, outputBuffer, numFrames);
            }
            else if (currentEffectMode == 2) {
                // Mode 2: Karaoke (Gema luas)
                echoKaraoke->process(outputBuffer, outputBuffer, numFrames);
                reverbSyahrini->process(outputBuffer, outputBuffer, numFrames);
            }
            else if (currentEffectMode == 3) {
                // Mode 3: Rapper TREASURE Vibe (Flanger + sedikit echo)
                efekTreasure->process(outputBuffer, outputBuffer, numFrames);
                echoKaraoke->process(outputBuffer, outputBuffer, numFrames);
            }
        }

        return oboe::DataCallbackResult::Continue;
    }
};

KaraokeEngine engine;

extern "C" JNIEXPORT void JNICALL Java_com_karaokego_MainActivity_startAudioEngine(JNIEnv* env, jobject) { engine.start(); }
extern "C" JNIEXPORT void JNICALL Java_com_karaokego_MainActivity_stopAudioEngine(JNIEnv* env, jobject) { engine.stop(); }
extern "C" JNIEXPORT void JNICALL Java_com_karaokego_MainActivity_setAudioMode(JNIEnv* env, jobject, jint mode) { engine.setMode(mode); }