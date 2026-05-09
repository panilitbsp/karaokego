#include <jni.h>
#include <oboe/Oboe.h>
#include <android/log.h>
#include <memory>

#include <Superpowered.h>
#include <SuperpoweredReverb.h>
#include <SuperpoweredEcho.h>

// [TAMBAHAN BARU] Import alat Equalizer dan Autotune Asli!
#include <Superpowered3BandEQ.h>
#include <SuperpoweredAutomaticVocalPitchCorrection.h>

#define LOG_TAG "MesinKaraoke"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class KaraokeEngine : public oboe::AudioStreamDataCallback {
private:
    std::shared_ptr<oboe::AudioStream> mRecordStream;
    std::shared_ptr<oboe::AudioStream> mPlayStream;

    int currentEffectMode = 0;

    // [SOLUSI CRASH] Pointer efek kita jadikan nilai awal kosong (nullptr)
    Superpowered::Reverb* reverbSyahrini = nullptr;
    Superpowered::Echo* echoKaraoke = nullptr;
    Superpowered::AutomaticVocalPitchCorrection* efekAutotune = nullptr;
    Superpowered::ThreeBandEQ* eqSyahrini = nullptr;

public:
    bool start() {
        // PASTE KUNCI ASLIMU DI SINI LAGI YA
        Superpowered::Initialize("S1pyd3VCajBabTFvWjRkMTE1YWE1ZWVhNDJjZmJlODVmYzJiNGFjNmMyNzhmMjRjN2YxNDhMZGsxREd6U3JNZmJnWmd2VEE0");

        oboe::AudioStreamBuilder recordBuilder;
        recordBuilder.setDirection(oboe::Direction::Input);
        recordBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        recordBuilder.setFormat(oboe::AudioFormat::Float);
        recordBuilder.setChannelCount(1);

        if (recordBuilder.openStream(mRecordStream) != oboe::Result::OK) return false;

        oboe::AudioStreamBuilder playBuilder;
        playBuilder.setDirection(oboe::Direction::Output);
        playBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        playBuilder.setFormat(oboe::AudioFormat::Float);
        playBuilder.setChannelCount(2);
        playBuilder.setDataCallback(this);

        if (playBuilder.openStream(mPlayStream) != oboe::Result::OK) return false;

        int sampleRate = mPlayStream->getSampleRate();

        // Kita hanya membuat efek SEKALI seumur hidup aplikasi biar nggak pernah crash
        if (reverbSyahrini == nullptr) {
            reverbSyahrini = new Superpowered::Reverb(sampleRate);
            reverbSyahrini->enabled = true;
            reverbSyahrini->mix = 0.15f; // [SOLUSI] Mix dikecilin biar suara nggak kebesaran
        }

        if (eqSyahrini == nullptr) {
            eqSyahrini = new Superpowered::ThreeBandEQ(sampleRate);
            eqSyahrini->enabled = true;
            eqSyahrini->low = 0.3f;   // [SOLUSI NADA CEWE] Potong Bass biar nggak berat!
            eqSyahrini->mid = 1.1f;   // Suara vokal tengah normal
            eqSyahrini->high = 1.8f;  // Naikkan Treble biar suara bening dan merdu
        }

        if (echoKaraoke == nullptr) {
            echoKaraoke = new Superpowered::Echo(sampleRate);
            echoKaraoke->enabled = true;
            echoKaraoke->setMix(0.3f);
        }

        if (efekAutotune == nullptr) {
            // [SOLUSI AUTOTUNE] Ini mesin Pitch Correction aslinya!
            efekAutotune = new Superpowered::AutomaticVocalPitchCorrection(sampleRate);
            efekAutotune->enabled = true;
        }

        mRecordStream->requestStart();
        mPlayStream->requestStart();
        LOGI("Mesin Oboe Berjalan!");
        return true;
    }

    void stop() {
        if (mPlayStream) { mPlayStream->requestStop(); mPlayStream->close(); mPlayStream.reset(); }
        if (mRecordStream) { mRecordStream->requestStop(); mRecordStream->close(); mRecordStream.reset(); }

        // ==============================================================
        // KITA TIDAK LAGI MENGHAPUS EFEK DI SINI! (No more `delete`)
        // Inilah kunci utama yang akan membasmi semua error/bug saat pindah mode
        // ==============================================================
        LOGI("Mesin Oboe Dimatikan.");
    }

    void setMode(int mode) {
        currentEffectMode = mode;
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        float* outputBuffer = static_cast<float*>(audioData);
        memset(outputBuffer, 0, numFrames * 2 * sizeof(float));

        if (mRecordStream) {
            float monoBuffer[numFrames];
            memset(monoBuffer, 0, numFrames * sizeof(float));
            mRecordStream->read(monoBuffer, numFrames, 0);

            // 1. PROSES EFEK MONO
            // Autotune WAJIB memproses suara mentah mono sebelum dipecah ke stereo
            if (currentEffectMode == 3 && efekAutotune != nullptr) {
                efekAutotune->process(monoBuffer, monoBuffer, numFrames);
            }

            // 2. Ubah Mono (Mic) ke Stereo (Kiri-Kanan)
            for(int i = 0; i < numFrames; i++) {
                outputBuffer[i*2]     = monoBuffer[i];
                outputBuffer[i*2 + 1] = monoBuffer[i];
            }

            // 3. PROSES EFEK STEREO
            if (currentEffectMode == 1) {
                // Mode Syahrini: Disaring lewat EQ (potong bass) dulu, baru dikasih Reverb!
                if (eqSyahrini) eqSyahrini->process(outputBuffer, outputBuffer, numFrames);
                if (reverbSyahrini) reverbSyahrini->process(outputBuffer, outputBuffer, numFrames);
            }
            else if (currentEffectMode == 2) {
                // Mode Karaoke
                if (echoKaraoke) echoKaraoke->process(outputBuffer, outputBuffer, numFrames);
                if (reverbSyahrini) reverbSyahrini->process(outputBuffer, outputBuffer, numFrames);
            }
            else if (currentEffectMode == 3) {
                // Mode Autotune: Vokal udah otomatis merdu, kita tambahin echo sedikit aja
                if (echoKaraoke) echoKaraoke->process(outputBuffer, outputBuffer, numFrames);
            }
        }

        return oboe::DataCallbackResult::Continue;
    }
};

KaraokeEngine engine;

extern "C" JNIEXPORT void JNICALL Java_com_karaokego_MainActivity_startAudioEngine(JNIEnv* env, jobject) { engine.start(); }
extern "C" JNIEXPORT void JNICALL Java_com_karaokego_MainActivity_stopAudioEngine(JNIEnv* env, jobject) { engine.stop(); }
extern "C" JNIEXPORT void JNICALL Java_com_karaokego_MainActivity_setAudioMode(JNIEnv* env, jobject, jint mode) { engine.setMode(mode); }