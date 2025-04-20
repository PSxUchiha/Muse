cmake_minimum_required(VERSION 3.16)
project(Muse VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets REQUIRED)
find_package(TagLib REQUIRED)

add_executable(muse
    main.cpp
    mainwindow.cpp
    mainwindow.h
    musiclibrary.cpp
    musiclibrary.h
)

target_link_libraries(muse PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    TagLib::TagLib
)
