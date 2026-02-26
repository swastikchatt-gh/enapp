cmake_minimum_required(VERSION 3.16)
project(MyFileManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

qt_standard_project_setup()

qt6_add_resources(RESOURCES resources.qrc)

add_executable(myfilemanager
    src/main.cpp
    ${RESOURCES}
)

target_link_libraries(myfilemanager Qt6::Core Qt6::Widgets Qt6::Gui)

install(TARGETS myfilemanager RUNTIME DESTINATION bin)
