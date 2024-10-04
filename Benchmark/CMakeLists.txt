cmake_minimum_required(VERSION 3.14)

project(Benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)
endif()


# === === === Put throughput  === === ===
# Source files for the benchmark
set(BENCHMARK_SRCS
        put_throughput_benchmark.cpp
)
# Add executable for the benchmark
add_executable(put_benchmark
        ${BENCHMARK_SRCS}
)
# Include directories for the benchmark executable
target_include_directories(put_benchmark PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)
# Link libraries to the benchmark executable
target_link_libraries(put_benchmark PRIVATE
        veloxdb_lib
)

#  === === === Get Latency === === ===
set(GET_BENCHMARK_SRCS
        get_latency_benchmark.cpp
)
# Add executable for Get latency benchmark
add_executable(get_benchmark ${GET_BENCHMARK_SRCS})

target_include_directories(get_benchmark PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)
# Link VeloxDB library to the get_benchmark executable
target_link_libraries(get_benchmark PRIVATE
        veloxdb_lib
)

# === === === Scan throughput  === === ===
# Source files for the benchmark
set(SCAN_BENCHMARK_SRCS
        scan_throughput_benchmark.cpp
)
# Add executable for the benchmark
add_executable(scan_benchmark
        ${SCAN_BENCHMARK_SRCS}
)
# Include directories for the benchmark executable
target_include_directories(scan_benchmark PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)
# Link libraries to the benchmark executable
target_link_libraries(scan_benchmark PRIVATE
        veloxdb_lib
)

