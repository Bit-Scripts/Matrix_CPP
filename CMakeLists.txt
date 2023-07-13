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

file(COPY ${CMAKE_SOURCE_DIR}/Matrix.png
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/share/icons)

file(COPY ${CMAKE_SOURCE_DIR}/Matrix.png
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/matrixresources/icons)

file(COPY ${CMAKE_SOURCE_DIR}/MatrixLogo.png
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/matrixresources/icons)

file(COPY ${CMAKE_SOURCE_DIR}/fonts/mtx.ttf
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/matrixresources/fonts)

INSTALL(TARGETS ${PROJECT_NAME}
        LIBRARY
        DESTINATION lib)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/Matrix.png
        DESTINATION share/icons)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/Matrix.png
        DESTINATION bin/matrixresources/icons)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/MatrixLogo.png
        DESTINATION bin/matrixresources/icons)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/fonts/mtx.ttf
        DESTINATION bin/matrixresources/fonts)

set(APP_DESKTOP_FILE "matrix.desktop")
configure_file(${CMAKE_SOURCE_DIR}/${APP_DESKTOP_FILE} ${CMAKE_BINARY_DIR}/${APP_DESKTOP_FILE} @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${APP_DESKTOP_FILE} DESTINATION share/applications)


INSTALL(TARGETS ${PROJECT_NAME}
        DESTINATION bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "Matrix")
set(CPACK_PACKAGE_VERSION "1.0.16")
set(CPACK_PACKAGE_VENDOR "Bit-Scripts")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Video processing with Matrix effect")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Matrix")
set(CPACK_PACKAGE_CONTACT "Paul WOISARD <paulwoisard@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6, libqt6core6, libqt6multimedia6, gstreamer1.0-opencv, libopencv-apps2d, libopencv-core406, libopencv-imgcodecs406, libopencv-imgproc406, libopencv-objdetect406, libopencv-photo406, libopencv-shape406, libopencv-video406, libopencv-videoio406, opencv-data, ffmpeg, gstreamer1.0-libav, v4l2loopback-utils, v4l2loopback-dkms")

# Include CPack module
include(CPack)        