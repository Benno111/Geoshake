cmake_minimum_required(VERSION 3.25)

project(Geoshake VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/Haptics.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    enable_language(OBJCXX)
    set_target_properties(${PROJECT_NAME} PROPERTIES OBJCXX_STANDARD 23 OBJCXX_STANDARD_REQUIRED ON)
    target_sources(${PROJECT_NAME} PRIVATE src/HapticsApple.mm)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Foundation" "-framework UIKit" "-framework AudioToolbox")
elseif(ANDROID)
    target_sources(${PROJECT_NAME} PRIVATE src/HapticsAndroid.cpp)
endif()

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Set GEODE_SDK to the path of a Geode SDK checkout")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
setup_geode_mod(${PROJECT_NAME})
