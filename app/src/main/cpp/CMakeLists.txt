# Versi minimum CMake
cmake_minimum_required(VERSION 3.22.1)

# Nama proyek C++ kita
project("karaokego")

# 1. Cari library Oboe yang sudah disiapkan oleh Prefab Gradle
find_package(oboe REQUIRED CONFIG)

# 2. Daftarkan file C++ kita
add_library(karaokego SHARED native-lib.cpp)

# 3. Gabungkan library log bawaan Android dan Oboe ke dalam proyek kita
target_link_libraries(karaokego
        log
        oboe::oboe)