cmake_minimum_required(VERSION 3.20)

project(DataStructuresAndAlgorithms LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dsa INTERFACE)
target_include_directories(dsa INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(dsa INTERFACE cxx_std_23)

add_executable(dsa_demo DataStructuresAndAlgorithms.cpp)
target_link_libraries(dsa_demo PRIVATE dsa)

include(CTest)
if(BUILD_TESTING)
    function(add_dsa_test target source)
        add_executable(${target} ${source})
        target_link_libraries(${target} PRIVATE dsa)
        add_test(NAME ${target} COMMAND ${target})
    endfunction()

    add_dsa_test(dsa_arrays_hashing_tests tests/arrays_hashing_tests.cpp)
    add_dsa_test(dsa_remaining_batch_a_tests tests/remaining_batch_a_tests.cpp)
    add_dsa_test(dsa_remaining_batch_b_tests tests/remaining_batch_b_tests.cpp)
    add_dsa_test(dsa_remaining_batch_c_tests tests/remaining_batch_c_tests.cpp)
    add_dsa_test(dsa_remaining_batch_d_tests tests/remaining_batch_d_tests.cpp)
endif()
