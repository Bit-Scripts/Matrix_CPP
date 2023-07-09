cmake_minimum_required(VERSION 3.19)
project(Matrix)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Widgets Multimedia REQUIRED)
find_package(OpenCV 4 REQUIRED)
find_package(FFmpeg REQUIRED)
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
        ${FFMPEG_LIBRARIES}
        )

target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/fonts
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts
        COMMENT "Copying font directory"
)

file(COPY ${CMAKE_SOURCE_DIR}/matrix-linux.png
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/MatrixLogo.png
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

INSTALL(TARGETS ${PROJECT_NAME}
        LIBRARY
        DESTINATION lib)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/matrix-linux.png
        DESTINATION bin)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/MatrixLogo.png
        DESTINATION bin)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/fonts/mtx.ttf
        DESTINATION bin/fonts)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/Matrix.desktop
        DESTINATION share/applications)

INSTALL(TARGETS ${PROJECT_NAME}
        DESTINATION bin)
