g++ -std=c++11 -o bin/my_program main.cpp `pkg-config --cflags --libs opencv4`
./bin

mkdir build && cd build.
cmake ..
cmake --build .
./misis2024s_21_03_aleseeev_a_r

cmake --build . && ./misis2024s_21_03_aleseeev_a_r
./misis2024s_21_03_aleseeev_a_r -s 3 -h 40 output_filename.jpg -gamma 5