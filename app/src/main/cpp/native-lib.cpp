#include <jni.h>
#include <oboe/Oboe.h>
#include <android/log.h>
#include <memory>

// Import alat make-up dari folder Superpowered!
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

    // Siapkan wadah untuk efek-efeknya
    Superpowered::Reverb* reverbSyahrini;
    Superpowered::Echo* echoKaraoke;
    Superpowered::Flanger* efekTreasure;

public:
    bool start() {
        // Nyalakan lisensi inti Superpowered (Wajib)
        Superpowered::Initialize("ExampleLicenseKey-WillWorkForAWhile", false, false, false, false, false, false, false);

        // --- Konfigurasi Mic (Input) ---
        oboe::AudioStreamBuilder recordBuilder;
        recordBuilder.setDirection(oboe::Direction::Input);
        recordBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        recordBuilder.setFormat(oboe::AudioFormat::Float);
        recordBuilder.setChannelCount(1); // Mic hp itu Mono

        if (recordBuilder.openStream(mRecordStream) != oboe::Result::OK) {
            return false;
        }

        // --- Konfigurasi Speaker (Output) ---
        oboe::AudioStreamBuilder playBuilder;
        playBuilder.setDirection(oboe::Direction::Output);
        playBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        playBuilder.setFormat(oboe::AudioFormat::Float);
        playBuilder.setChannelCount(2); // Wajib Stereo biar efeknya keluar dari headset kiri-kanan
        playBuilder.setDataCallback(this);

        if (playBuilder.openStream(mPlayStream) != oboe::Result::OK) {
            return false;
        }

        int sampleRate = mPlayStream->getSampleRate();

        // 1. Setup Efek Syahrini
        reverbSyahrini = new Superpowered::Reverb(sampleRate);
        reverbSyahrini->enabled = true;
        reverbSyahrini->mix = 0.3f; // UBAH setMix menjadi mix

        // 2. Setup Efek Karaoke
        echoKaraoke = new Superpowered::Echo(sampleRate);
        echoKaraoke->enabled = true;
        echoKaraoke->mix = 0.4f; // UBAH setMix menjadi mix

        // 3. Setup Efek Swag Rapper TREASURE (Digital Flanger)
        // Karena autotune asli butuh set kunci nada (Do Re Mi),
        // kita pakai efek Flanger biar suaranya jadi ada unsur elektronik/swag!
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

        // Bersihkan memori efek biar hp nggak lemot
        delete reverbSyahrini;
        delete echoKaraoke;
        delete efekTreasure;
        LOGI("Mesin Dimatikan.");
    }

    void setMode(int mode) {
        currentEffectMode = mode;
    }

    // Di sinilah suara diproses jutaan kali per detik
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        float* outputBuffer = static_cast<float*>(audioData);
        memset(outputBuffer, 0, numFrames * 2 * sizeof(float)); // Bersihkan memori output (dikali 2 karena Stereo)

        if (mRecordStream) {
            // Wadah kecil nampung suara mentah dari Mic
            float monoBuffer[numFrames];
            mRecordStream->read(monoBuffer, numFrames, 0);

            // Trik Ubah Mono ke Stereo: Isi telinga kiri dan kanan dengan suara yang sama
            for(int i = 0; i < numFrames; i++) {
                outputBuffer[i*2]     = monoBuffer[i]; // Kiri
                outputBuffer[i*2 + 1] = monoBuffer[i]; // Kanan
            }

            // === PABRIK MAKE-UP SUARA ===
            if (currentEffectMode == 1) {
                // Mode 1: Syahrini (Empuk elegan)
                reverbSyahrini->process(outputBuffer, outputBuffer, numFrames);
            }
            else if (currentEffectMode == 2) {
                // Mode 2: Karaoke (Gema luas banget)
                echoKaraoke->process(outputBuffer, outputBuffer, numFrames);
                reverbSyahrini->process(outputBuffer, outputBuffer, numFrames);
            }
            else if (currentEffectMode == 3) {
                // Mode 3: Rapper Vibe (Suara swag, agak robotic/electronic)
                efekTreasure->process(outputBuffer, outputBuffer, numFrames);
                echoKaraoke->process(outputBuffer, outputBuffer, numFrames);
            }
            // Kalau mode 0 (Original), biarkan suara langsung lewat tanpa diproses
        }

        return oboe::DataCallbackResult::Continue;
    }
};

KaraokeEngine engine;

extern "C" JNIEXPORT void JNICALL Java_com_karaokego_MainActivity_startAudioEngine(JNIEnv* env, jobject) { engine.start(); }
extern "C" JNIEXPORT void JNICALL Java_com_karaokego_MainActivity_stopAudioEngine(JNIEnv* env, jobject) { engine.stop(); }
extern "C" JNIEXPORT void JNICALL Java_com_karaokego_MainActivity_setAudioMode(JNIEnv* env, jobject, jint mode) { engine.setMode(mode); }