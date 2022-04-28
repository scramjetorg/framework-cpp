cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(
  IFCA
  VERSION 1.0
  HOMEPAGE_URL "https://github.com/scramjetorg/framework-cpp"
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(LOG_LVL LOG_LEVEL_NONE CACHE STRING  "Logging level used <LOG_LEVEL_ALL|LOG_LEVEL_DEBUG|LOG_LEVEL_INFO|LOG_LEVEL_WARNING|LOG_LEVEL_ERROR|LOG_LEVEL_NONE>" )
set(LOG_TYPE LOG_TYPE_CONSOLE CACHE STRING "Logging output used <LOG_TYPE_CONSOLE|LOG_TYPE_FILE>")
add_compile_definitions(LOG_LVL=${LOG_LVL} LOG_TYPE=${LOG_TYPE})

# TODO: add install
#option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)
option(BUILD_TESTING "Enable testing" OFF)

add_library(IFCA SHARED "")

set_target_properties(IFCA PROPERTIES CXX_STANDARD 17)

add_subdirectory(include)
add_subdirectory(src)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(IFCA PRIVATE Threads::Threads)

target_compile_options(IFCA PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

target_include_directories(
  IFCA PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

if(BUILD_TESTING OR ENABLE_TEST_COVERAGE)
    enable_testing()
    add_subdirectory(tests)
endif()