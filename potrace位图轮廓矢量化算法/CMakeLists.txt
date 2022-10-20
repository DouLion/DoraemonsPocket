cmake_minimum_required(VERSION 3.15)

project(potrace)
set(CMAKE_CXX_STANDARD 17)

find_path(GETOPT_INCLUDE_DIR getopt.h)
set(GETOPT_NAMES ${JRTPLIB_NAMES} getopt)
find_library(GETOPT_LIBRARY NAMES ${GETOPT_NAMES})
find_package( OpenCV 4 REQUIRED )
find_package(jsoncpp CONFIG REQUIRED)
    

add_executable(${PROJECT_NAME} example.cpp auxiliary.h backend_geojson.cpp backend_geojson.h
 bbox.cpp bbox.h bitmap.h bitmap_io.cpp bitmap_io.h bitops.h curve.cpp curve.h 
 decompose.cpp decompose.h flate.cpp flate.h greymap.cpp greymap.h lists.h 
 lzw.cpp lzw.h platform.h potracelib.cpp potracelib.h progress.h progress_bar.cpp
 progress_bar.h render.cpp render.h trace.cpp trace.h trans.cpp trans.h)
target_link_libraries(${PROJECT_NAME} opencv_core opencv_imgcodecs opencv_imgproc)
target_link_libraries(${PROJECT_NAME} ${GETOPT_LIBRARY})
target_link_libraries(${PROJECT_NAME} jsoncpp_lib jsoncpp_object JsonCpp::JsonCpp)