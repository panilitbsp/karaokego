#include <jni.h>
#include <oboe/Oboe.h>
#include <android/log.h>

#define LOG_TAG "MesinKaraoke"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Class Engine yang mengatur lalu lintas masuk/keluarnya suara
class KaraokeEngine : public oboe::AudioStreamDataCallback {
private:
    std::shared_ptr<oboe::AudioStream> mRecordStream;
    std::shared_ptr<oboe::AudioStream> mPlayStream;

public:
    bool start() {
        // 1. Konfigurasi Jalur Input (Mic)
        oboe::AudioStreamBuilder recordBuilder;
        recordBuilder.setDirection(oboe::Direction::Input)
                ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
                ->setFormat(oboe::AudioFormat::Float)
                ->setChannelCount(1); // Mono

        oboe::Result result = recordBuilder.openStream(mRecordStream);
        if (result != oboe::Result::OK) {
            LOGE("Gagal buka mic!");
            return false;
        }

        // 2. Konfigurasi Jalur Output (Speaker/Headphone)
        oboe::AudioStreamBuilder playBuilder;
        playBuilder.setDirection(oboe::Direction::Output)
                ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
                ->setFormat(oboe::AudioFormat::Float)
                ->setChannelCount(1) // Mono
                ->setDataCallback(this); // Arahkan proses datanya ke fungsi onAudioReady di bawah

        result = playBuilder.openStream(mPlayStream);
        if (result != oboe::Result::OK) {
            LOGE("Gagal buka speaker!");
            return false;
        }

        // 3. Nyalakan mesin!
        mRecordStream->requestStart();
        mPlayStream->requestStart();
        LOGI("Mesin Oboe Berjalan Sempurna!");
        return true;
    }

    void stop() {
        // Matikan jalur secara aman untuk menghindari memory leak
        if (mPlayStream) {
            mPlayStream->requestStop();
            mPlayStream->close();
        }
        if (mRecordStream) {
            mRecordStream->requestStop();
            mRecordStream->close();
        }
        LOGI("Mesin Oboe Dimatikan.");
    }

    // Fungsi ini dipanggil berulang-ulang dalam hitungan milidetik oleh sistem
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {

        // Tarik suara mentah dari Mic
        if (mRecordStream) {
            auto result = mRecordStream->read(audioData, numFrames, 0);
            if (result.value() != numFrames) {
                LOGE("Buffer underrun (suara putus-putus)");
            }
        }

        // (NANTINYA: Di baris ini kita akan menyisipkan algoritma Autotune / Efek Vokal sebelum suaranya keluar)

        // Suara yang ada di 'audioData' otomatis terlempar ke Speaker oleh Oboe
        return oboe::DataCallbackResult::Continue;
    }
};

// Buat satu wujud nyata dari mesin kita di memori
KaraokeEngine engine;

// Jembatan JNI untuk Tombol ON di Java
extern "C" JNIEXPORT void JNICALL
Java_com_karaokego_MainActivity_startAudioEngine(JNIEnv* env, jobject /* this */) {
    engine.start();
}

// Jembatan JNI untuk Tombol OFF di Java
extern "C" JNIEXPORT void JNICALL
Java_com_karaokego_MainActivity_stopAudioEngine(JNIEnv* env, jobject /* this */) {
    engine.stop();
}