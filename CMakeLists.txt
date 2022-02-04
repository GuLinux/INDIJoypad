cmake_minimum_required(VERSION 3.5)

project(INDIJoypad VERSION 0.1 LANGUAGES CXX)
include(FindPkgConfig)



set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network Widgets REQUIRED)
find_package(yaml-cpp REQUIRED)

pkg_check_modules(LIBINDI REQUIRED libindi)
include_directories(
    ${LIBINDI_INCLUDE_DIRS}
    ${YAML_CPP_INCLUDE_DIR}
)

set(CORE_SOURCES
    joystickdriver.cpp
    indiclient.cpp
    telescope.cpp
    settings.cpp
    mappings.cpp
    indidevice.cpp
    focuser.cpp
)

set(GUI_PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${CORE_SOURCES}
)

set(CLI_PROJECT_SOURCES
    main-cli.cpp
    ${CORE_SOURCES}
)

add_executable(INDIJoypad ${GUI_PROJECT_SOURCES})
add_executable(INDIJoypad-cli ${CLI_PROJECT_SOURCES})

set(COMMON_LIBRARIES Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network pthread ${LIBINDI_LIBRARIES} ${YAML_CPP_LIBRARIES} -lindiclientqt)

target_link_libraries(INDIJoypad PRIVATE ${COMMON_LIBRARIES})
target_link_libraries(INDIJoypad-cli PRIVATE ${COMMON_LIBRARIES})

