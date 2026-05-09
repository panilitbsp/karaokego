package com.karaokego;

import androidx.appcompat.app.AlertDialog;
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

    // --- Jembatan ke Mesin C++ ---
    public native void startAudioEngine();
    public native void stopAudioEngine();
    public native void setAudioMode(int mode); // Ini buat ngirim pilihan mode ke C++

    private TextView tvStatus;
    private ImageView ivMic, btnSettings;

    private boolean isMicOn = false;
    private int modeTerpilih = 0; // 0: Ori, 1: Syahrini, 2: Karaoke, 3: Treasure

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tvStatus = findViewById(R.id.tvStatus);
        ivMic = findViewById(R.id.ivMic);
        btnSettings = findViewById(R.id.btnSettings);

        mintaIzinMic();

        // 1. Logika Klik Tombol Gear (Pengaturan Mode)
        btnSettings.setOnClickListener(v -> bukaMenuPilihanEfek());

        // 2. Logika Klik Tombol Mic (On/Off)
        ivMic.setOnClickListener(v -> {
            if (!cekkIzinMic()) {
                mintaIzinMic();
                return;
            }

            isMicOn = !isMicOn;

            if (isMicOn) {
                nyalakanMicUI();
                startAudioEngine();
            } else {
                matikanMicUI();
                stopAudioEngine();
            }
        });
    }

    // --- Fungsi buat nampilin Dialog Pilihan Mode ---
    private void bukaMenuPilihanEfek() {
        String[] listMode = {
                "🎙️ Mode Original",
                "✨ Mode Syahrini (Percantik)",
                "🎤 Mode Karaoke (Gema Luas)",
                "😎 Mode Autotune (Vibe Rapper TREASURE)"
        };

        // UBAH MENJADI (Hapus koma dan tulisan R.style-nya):
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Pilih Efek Suara");

        // Menampilkan pilihan (Radio Button)
        builder.setSingleChoiceItems(listMode, modeTerpilih, (dialog, which) -> {
            modeTerpilih = which;

            // Langsung kirim angkanya ke C++ (0, 1, 2, atau 3)
            setAudioMode(modeTerpilih);

            Toast.makeText(this, "Efek diganti ke: " + listMode[which], Toast.LENGTH_SHORT).show();
            dialog.dismiss();
        });

        builder.show();
    }

    private void nyalakanMicUI() {
        tvStatus.setText("MIC ON \uD83C\uDFA4");
        tvStatus.setTextColor(Color.parseColor("#00E676"));
        ivMic.animate().scaleX(1.1f).scaleY(1.1f).setDuration(200).start();
    }

    private void matikanMicUI() {
        tvStatus.setText("MIC OFF");
        tvStatus.setTextColor(Color.parseColor("#555555"));
        ivMic.animate().scaleX(1.0f).scaleY(1.0f).setDuration(200).start();
    }

    private boolean cekkIzinMic() {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED;
    }

    private void mintaIzinMic() {
        if (!cekkIzinMic()) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.RECORD_AUDIO}, PERMISSION_RECORD_AUDIO);
        }
    }

    static {
        System.loadLibrary("karaokego");
    }
}