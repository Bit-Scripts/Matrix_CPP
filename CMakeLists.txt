set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

cmake_minimum_required(VERSION 3.19)
project(Matrix)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_BUILD_TYPE Release)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)
find_package(OpenCV 4 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCES
        main.cpp
        mainwindow.cpp mainwindow.h
        cameracapture.cpp cameracapture.h
        videoprocesssing.cpp videoprocesssing.h
        raindrop.cpp raindrop.h
        virtualcamera.cpp virtualcamera.h)

set(RC_SRC
        resources.qrc)

qt_add_resources(${PROJECT_NAME} ${RC_SRC})

qt_add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        ${OpenCV_LIBS}
        xcb
        )

find_path( AVCODEC_INCLUDE_DIR libavcodec/avcodec.h )
find_library( AVCODEC_LIBRARY avcodec )
target_include_directories( ${PROJECT_NAME} PRIVATE ${AVCODEC_INCLUDE_DIR} )
target_link_libraries( ${PROJECT_NAME} PRIVATE ${AVCODEC_LIBRARY})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/fonts
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts
        COMMENT "Copying font directory"
)

file(COPY ${CMAKE_SOURCE_DIR}/com.bitscripts.matrix.png
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_SOURCE_DIR}/MatrixLogo.png
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

INSTALL(TARGETS ${PROJECT_NAME}
        LIBRARY
        DESTINATION lib)

execute_process(COMMAND bash "-c" "printenv | grep FLATPAK_ID"
OUTPUT_VARIABLE IS_FLATPAK)

if(NOT IS_FLATPAK)
        INSTALL(FILES ${CMAKE_SOURCE_DIR}/com.bitscripts.matrix.png
                DESTINATION bin)

        INSTALL(FILES ${CMAKE_SOURCE_DIR}/MatrixLogo.png
                DESTINATION bin)

        INSTALL(FILES ${CMAKE_SOURCE_DIR}/fonts/mtx.ttf
                DESTINATION bin/fonts)

        INSTALL(FILES ${CMAKE_SOURCE_DIR}/com.bitscripts.matrix.desktop
                DESTINATION share/applications)
endif()

INSTALL(TARGETS ${PROJECT_NAME}
        DESTINATION bin)
