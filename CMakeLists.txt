cmake_minimum_required(VERSION 3.17)
project(im_view)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 6)
set(REQUIRED_LIBS Core Gui Widgets Test)
set(OPTIONAL_LIBS PrintSupport)
set(REQUIRED_LIBS_QUALIFIED Qt6::Core Qt6::Gui Qt6::Widgets)
set(OPTIONAL_LIBS_QUALIFIED Qt6::PrintSupport)

set(LIB_SOURCES include/FileIterator.h main/AsyncFileIterator.h main/AsyncFileIterator.cpp
        main/FileIteratorState.h main/FileIteratorState.cpp
        include/utils.h main/utils.cpp)

include_directories(include)

if (NOT CMAKE_PREFIX_PATH)
    message(WARNING "CMAKE_PREFIX_PATH is not defined, you may need to set it "
            "(-DCMAKE_PREFIX_PATH=\"path/to/Qt/lib/cmake\" or -DCMAKE_PREFIX_PATH=/usr/include/{host}/qt{version}/ on Ubuntu)")
endif ()

add_executable(${PROJECT_NAME} main.cpp ${LIB_SOURCES} main/OpenEventApp.h main/ImageViewer.h main/ImageViewer.cpp
        main/HandDragScrollArea.h main/HandDragScrollArea.cpp)
add_library(${PROJECT_NAME}_LIB SHARED ${LIB_SOURCES})

find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
find_package(Qt${QT_VERSION} COMPONENTS ${OPTIONAL_LIBS} QUIET)

if(Qt${QT_VERSION}::PrintSupport_FOUND)
    target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED} ${OPTIONAL_LIBS_QUALIFIED})
    target_link_libraries(${PROJECT_NAME}_LIB ${REQUIRED_LIBS_QUALIFIED} ${OPTIONAL_LIBS_QUALIFIED})
else()
    target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})
    target_link_libraries(${PROJECT_NAME}_LIB ${REQUIRED_LIBS_QUALIFIED})
endif()

add_executable(iteratorTest test/AsyncFileIteratorTest.cpp)
add_test(NAME iteratorTest COMMAND iteratorTest)
target_link_libraries(iteratorTest PRIVATE ${PROJECT_NAME}_LIB Qt${QT_VERSION}::Test)

add_executable(utilsTest test/UtilsTest.cpp)
add_test(NAME utilsTest COMMAND utilsTest)
target_link_libraries(utilsTest PRIVATE ${PROJECT_NAME}_LIB Qt${QT_VERSION}::Test)