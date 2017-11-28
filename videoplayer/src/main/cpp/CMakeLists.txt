#Cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

#添加库
add_library( # 为库指定名称.
             FileScanner

             # 指定库类型.
             SHARED

             # 库文件地址.
             FilesScanner.cpp )

#引入NDK自带的库
find_library( log-lib

              # ndk lib库的名称
              log )

target_link_libraries( # Specifies the target library.
                       FileScanner

                       # Links the log library to the target library.
                       ${log-lib} )