# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html


cmake_minimum_required(VERSION 3.22.1)


project("openglstudy")


AUX_SOURCE_DIRECTORY(${CMAKE_SOURCE_DIR} SRC_LIST)

add_library( # Sets the name of the library.
        openglstudy

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${SRC_LIST})


find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)


target_link_libraries( # Specifies the target library.
        openglstudy

        # OpenGL es依赖库
        GLESv3
        # EGL依赖库
        EGL
        android
        #操作Bitmap库
        jnigraphics
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})