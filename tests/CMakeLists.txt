
option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

find_package(doctest REQUIRED)
find_package(Threads REQUIRED)

#if(TEST_INSTALLED_VERSION)
#  find_package(IFCA REQUIRED)
#endif()

set(testExecutable "IFCA_tests")

add_executable(${testExecutable} "")
target_sources(${testExecutable} PUBLIC
    "ifca/transform/each.test.cpp"
    "ifca/transform/filter.test.cpp"
    "ifca/transform/from.test.cpp"
    "ifca/transform/into.test.cpp"
    "ifca/transform/map.test.cpp"
    "ifca/transform/race.test.cpp"
    "ifca/transform/reduce.test.cpp"
    "ifca/transform/run.test.cpp"
    
    "ifca/drain.test.cpp"
    "ifca/ifca.test.cpp"
    
    "scenarios/ifca.test.cpp"
    "scenarios/testData.hpp"

    "streams/dataStream.test.cpp"

    "main.cpp"
    "testClass.hpp"
)

target_link_libraries(${testExecutable} PUBLIC IFCA Threads::Threads doctest::doctest)
target_compile_features(${testExecutable} PRIVATE cxx_std_17 )

target_include_directories(${testExecutable} PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    ${DOCTEST_INCLUDE_DIR}
)

 if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${testExecutable} PUBLIC -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(${testExecutable} PUBLIC /W4 /WX /std:c++17)
    target_compile_definitions(IFCA PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()

if(ENABLE_TEST_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(${testExecutable} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(${testExecutable} PUBLIC -fprofile-arcs -ftest-coverage -lgcov)
endif()
