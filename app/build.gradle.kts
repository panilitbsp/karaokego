plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.karaokego"
    compileSdk = 35

    defaultConfig {
        applicationId = "com.karaokego"
        minSdk = 24
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        // === TAMBAHKAN BLOK INI ===
        externalNativeBuild {
            cmake {
                // Memerintahkan compiler untuk menggunakan standar C++17
                cppFlags += "-std=c++17"
            }
        }
        // ==========================
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
    buildFeatures {
        viewBinding = true
        prefab = true // Tambahkan baris ini wajib untuk Oboe
    }
}

dependencies {

    implementation("com.google.oboe:oboe:1.8.0")

    implementation(libs.appcompat)
    implementation(libs.material)
    implementation(libs.constraintlayout)
    testImplementation(libs.junit)
    androidTestImplementation(libs.ext.junit)
    androidTestImplementation(libs.espresso.core)
}