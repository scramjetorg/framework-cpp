add_executable(KM3 "")

target_sources(KM3 PUBLIC "main.cpp")

target_link_libraries(KM3 INTERFACE IFCA PUBLIC Threads::Threads)
target_compile_features(KM3 PRIVATE cxx_std_17 )

target_include_directories(KM3 PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

 if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(KM3 PUBLIC -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(KM3 PUBLIC /W4 /WX /std:c++17)
  endif()