cmake_minimum_required(VERSION 3.4.1)

set(libs_DIR ${CMAKE_SOURCE_DIR}/../../../Platinum-libs/${ANDROID_ABI})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

add_library(Platinum STATIC IMPORTED)
set_target_properties(Platinum PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/libPlatinum.a)

add_library(Neptune STATIC IMPORTED)
set_target_properties(Neptune PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/libNeptune.a)

add_library(axTLS STATIC IMPORTED)
set_target_properties(axTLS PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/libaxTLS.a)

add_library(MediaServer STATIC IMPORTED)
set_target_properties(MediaServer PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/libPltMediaServer.a)

add_library(MediaRender STATIC IMPORTED)
set_target_properties(MediaRender PROPERTIES IMPORTED_LOCATION
    ${libs_DIR}/libPltMediaRenderer.a)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -UNDEBUG")

add_library(platinum-jni SHARED
            PltMicroMediaController.cpp
            PltMicroMediaRenderer.cpp
            PltMicroMediaServer.cpp
            MediaRemote_notify.cpp
            MediaRemote.cpp
            MediaRemote_jni.cpp)

target_include_directories(platinum-jni PRIVATE
                           include/Platinum
                           include/Core
                           include/Devices/MediaConnect
                           include/Devices/MediaServer
                           include/Devices/MediaRenderer
                           include/Extras
                           include/Neptune/Source/Core)

target_link_libraries(platinum-jni
                      android
                      MediaServer
                      MediaRender
                      Platinum
                      Neptune
                      axTLS
                      log)