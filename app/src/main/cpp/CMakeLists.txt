# Versi minimum CMake
cmake_minimum_required(VERSION 3.22.1)

# Nama proyek C++ kita
project("karaokego")

find_package(oboe REQUIRED CONFIG)

# 1. Kenalkan folder Superpowered ke compiler
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Superpowered)

# 2. Hubungkan library Superpowered khusus untuk HP Android
add_library(Superpowered STATIC IMPORTED)
set_target_properties(Superpowered PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/Superpowered/lib/libSuperpoweredAndroid${ANDROID_ABI}.a)

# 3. Daftarkan file C++ kita
add_library(karaokego SHARED native-lib.cpp)

# 4. Gabungkan Oboe dan Superpowered ke dalam aplikasi kita
target_link_libraries(karaokego
        log
        oboe::oboe
        Superpowered) # <--- Ini yang bikin C++ kamu jadi sakti