cmake_minimum_required(VERSION 3.27)
project(misis2024s_21_03_aleseeev_a_r)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(misis2024s_21_03_aleseeev_a_r
        prj.lab/lab01/main.cpp)

target_link_libraries(misis2024s_21_03_aleseeev_a_r ${OpenCV_LIBS})