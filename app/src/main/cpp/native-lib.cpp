#include <jni.h>
#include <oboe/Oboe.h>
#include <android/log.h>
#include <memory> // TAMBAHAN WAJIB agar std::shared_ptr tidak error

#define LOG_TAG "MesinKaraoke"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class KaraokeEngine : public oboe::AudioStreamDataCallback {
private:
    std::shared_ptr<oboe::AudioStream> mRecordStream;
    std::shared_ptr<oboe::AudioStream> mPlayStream;

public:
    bool start() {
        // Konfigurasi Input
        oboe::AudioStreamBuilder recordBuilder;
        recordBuilder.setDirection(oboe::Direction::Input);
        recordBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        recordBuilder.setFormat(oboe::AudioFormat::Float);
        recordBuilder.setChannelCount(1);

        oboe::Result result = recordBuilder.openStream(mRecordStream);
        if (result != oboe::Result::OK) {
            LOGE("Gagal buka mic!");
            return false;
        }

        // Konfigurasi Output
        oboe::AudioStreamBuilder playBuilder;
        playBuilder.setDirection(oboe::Direction::Output);
        playBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        playBuilder.setFormat(oboe::AudioFormat::Float);
        playBuilder.setChannelCount(1);
        playBuilder.setDataCallback(this);

        result = playBuilder.openStream(mPlayStream);
        if (result != oboe::Result::OK) {
            LOGE("Gagal buka speaker!");
            return false;
        }

        mRecordStream->requestStart();
        mPlayStream->requestStart();
        LOGI("Mesin Oboe Berjalan Sempurna!");
        return true;
    }

    void stop() {
        if (mPlayStream) {
            mPlayStream->requestStop();
            mPlayStream->close();
            mPlayStream.reset(); // Bersihkan memori dengan aman
        }
        if (mRecordStream) {
            mRecordStream->requestStop();
            mRecordStream->close();
            mRecordStream.reset(); // Bersihkan memori dengan aman
        }
        LOGI("Mesin Oboe Dimatikan.");
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        if (mRecordStream) {
            auto result = mRecordStream->read(audioData, numFrames, 0);
            if (result == oboe::Result::OK) {
                int framesRead = result.value();
                // Jika mic telat mengirim data, isi sisa ruang dengan 0 agar suara tidak menjadi bunyi kresek-kresek
                if (framesRead < numFrames) {
                    int bytesPerFrame = audioStream->getBytesPerFrame();
                    memset(static_cast<uint8_t*>(audioData) + (framesRead * bytesPerFrame), 0, (numFrames - framesRead) * bytesPerFrame);
                }
            }
        }
        return oboe::DataCallbackResult::Continue;
    }
};

KaraokeEngine engine;

extern "C" JNIEXPORT void JNICALL
Java_com_karaokego_MainActivity_startAudioEngine(JNIEnv* env, jobject /* this */) {
engine.start();
}

extern "C" JNIEXPORT void JNICALL
Java_com_karaokego_MainActivity_stopAudioEngine(JNIEnv* env, jobject /* this */) {
engine.stop();
}