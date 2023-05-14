cmake_minimum_required(VERSION 3.10)

project(lang VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Output final binaries to the 'bin' folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(lang src/lang.cpp src/cpm/cpm.cpp src/cpm/base_distribution.cpp src/cpm/copy_pointer_manager.cpp src/cpm/copy_pointer_threshold.cpp)

