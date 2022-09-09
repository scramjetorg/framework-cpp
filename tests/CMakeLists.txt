
option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
#option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

find_package(doctest REQUIRED)
find_package(Threads REQUIRED)

#if(TEST_INSTALLED_VERSION)
#  find_package(IFCA REQUIRED)
#endif()

set(testExecutable "IFCA_tests")

add_executable(${testExecutable} "")
target_sources(${testExecutable} PUBLIC
    "main.cpp"
    "testClass.hpp"
)

add_subdirectory(ifca)
add_subdirectory(scenarios)
add_subdirectory(streams)

target_link_libraries(${testExecutable} INTERFACE IFCA PUBLIC Threads::Threads doctest::doctest)
target_compile_features(${testExecutable} PRIVATE cxx_std_17 )

target_include_directories(${testExecutable} PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    ${DOCTEST_INCLUDE_DIR}
)

 if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${testExecutable} PUBLIC -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(${testExecutable} PUBLIC /W4 /WX /std:c++17)
    target_compile_definitions(IFCA  INTERFACE DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()

if(ENABLE_TEST_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/cmake")
    include(CodeCoverage)
    append_coverage_compiler_flags()
    
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    setup_target_for_coverage_lcov(
      NAME IFCA_coverage 
      EXECUTABLE ${testExecutable} -j 8 
      DEPENDENCIES ${testExecutable}
      EXCLUDE  "/usr/*" "${PROJECT_SOURCE_DIR}/tests/*"
      )
endif()
