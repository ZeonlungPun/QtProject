cmake_minimum_required(VERSION 3.5)

project(cv2c VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ONNXRUNTIME_DIR "/home/kingargroo/cpp/onnxruntimeYOLOV8OBB/onnxruntime-linux-x64-1.16.3")
set(Torch_DIR "/home/kingargroo/cpp/torch1/libtorch/share/cmake/Torch")
find_package(Torch REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(OpenCV 4 REQUIRED)


set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(cv2c
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET cv2c APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(cv2c SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(cv2c
            ${PROJECT_SOURCES}
        )
    endif()
endif()

include_directories(${OpenCV_INCLUDE_DIRS} )
include_directories("${ONNXRUNTIME_DIR}/include")
target_link_libraries(cv2c PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${OpenCV_LIBS} "${ONNXRUNTIME_DIR}/lib/libonnxruntime.so" "${TORCH_LIBRARIES}")

set_target_properties(cv2c PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(cv2c)
endif()
