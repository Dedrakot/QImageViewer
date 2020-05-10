cmake_minimum_required(VERSION 3.5)

project(ImageViewer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS PrintSupport QUIET)
find_package(Qt5 COMPONENTS Test REQUIRED)
enable_testing(true)

add_executable(${PROJECT_NAME}
        src/main/imageviewer.cpp
        src/main/imageviewer.h
        src/main/main.cpp src/main/ImageIterator.cpp src/main/ImageIterator.h src/main/IteratorState.h src/main/IteratorState.cpp)

add_library(${PROJECT_NAME}_LIB
        SHARED
        src/main/imageviewer.cpp
        src/main/imageviewer.h
        src/main/ImageIterator.cpp src/main/ImageIterator.h src/main/IteratorState.h src/main/IteratorState.cpp)

if(Qt5::PrintSupport_FOUND)
  target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::PrintSupport)
  target_link_libraries(${PROJECT_NAME}_LIB Qt5::Widgets Qt5::PrintSupport)
else()
  target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
  target_link_libraries(${PROJECT_NAME}_LIB Qt5::Widgets)
endif()

add_executable(mytest src/test/test.cpp)
add_test(NAME mytest COMMAND mytest)

target_link_libraries(mytest PRIVATE Qt5::Test) # ${PROJECT_NAME}_LIB)
