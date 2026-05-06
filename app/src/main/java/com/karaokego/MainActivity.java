package com.karaokego;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private static final int PERMISSION_RECORD_AUDIO = 1;

    // Deklarasi fungsi JNI C++
    public native void startAudioEngine();
    public native void stopAudioEngine();

    private TextView tvStatus;
    private ImageView ivMic;

    // Variabel untuk menyimpan status Mic (Awalnya mati / false)
    private boolean isMicOn = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tvStatus = findViewById(R.id.tvStatus);
        ivMic = findViewById(R.id.ivMic);

        // Langsung todong izin mic saat aplikasi dibuka
        mintaIzinMic();

        // Logika saat gambar Mic ditekan
        ivMic.setOnClickListener(v -> {
            // Cek izin dulu
            if (!cekkIzinMic()) {
                Toast.makeText(this, "Kasih izin mic dulu dong!", Toast.LENGTH_SHORT).show();
                mintaIzinMic();
                return; // Stop proses kalau belum ada izin
            }

            // Balikkan status (kalau false jadi true, kalau true jadi false)
            isMicOn = !isMicOn;

            if (isMicOn) {
                // UI: Ubah jadi ON
                tvStatus.setText("MIC ON \uD83C\uDFA4");
                tvStatus.setTextColor(Color.parseColor("#00E676")); // Hijau neon

                // Animasi membesar sedikit (Skala 1.05x)
                ivMic.animate().scaleX(1.05f).scaleY(1.05f).setDuration(150).start();

                // Nyalakan mesin C++
                startAudioEngine();
            } else {
                // UI: Ubah jadi OFF
                tvStatus.setText("MIC OFF");
                tvStatus.setTextColor(Color.parseColor("#555555")); // Abu-abu redup

                // Animasi kembali ke ukuran normal (Skala 1.0x)
                ivMic.animate().scaleX(1.0f).scaleY(1.0f).setDuration(150).start();

                // Matikan mesin C++
                stopAudioEngine();
            }
        });
    }

    private boolean cekkIzinMic() {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO)
                == PackageManager.PERMISSION_GRANTED;
    }

    private void mintaIzinMic() {
        if (!cekkIzinMic()) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.RECORD_AUDIO},
                    PERMISSION_RECORD_AUDIO);
        }
    }

    static {
        System.loadLibrary("karaokego");
    }
}