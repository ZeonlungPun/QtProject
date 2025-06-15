cmake_minimum_required(VERSION 3.5)
project(face_attendence_multipool LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 正確設定 Qt5，避免 Qt4 被誤用
find_package(Qt5 COMPONENTS Widgets Sql REQUIRED)

# OpenCV 手動指定目錄
find_package(OpenCV 4 REQUIRED NO_DEFAULT_PATH
    PATHS /home/punzeonlung/cvDemo/opencv480-linux/lib/cmake/opencv4)

add_executable(face_attendence_multipool
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    childwindow.h
    childwindow.cpp
    childwindow.ui
)

target_link_libraries(face_attendence_multipool
    PRIVATE Qt5::Widgets Qt5::Sql ${OpenCV_LIBS}
)
