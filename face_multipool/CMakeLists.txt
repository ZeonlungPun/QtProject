cmake_minimum_required(VERSION 3.5)

project(face_multipool LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(Qt5 COMPONENTS Widgets Sql Core REQUIRED)
find_package(OpenCV 4 REQUIRED NO_DEFAULT_PATH
    PATHS /home/punzeonlung/cvDemo/opencv480-linux/lib/cmake/opencv4)

add_executable(face_multipool
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    childwindow.h
    childwindow.cpp
    childwindow.ui
    thread.cpp
    thread.h
)

target_link_libraries(face_multipool PRIVATE Qt5::Widgets Qt5::Sql ${OpenCV_LIBS} )
