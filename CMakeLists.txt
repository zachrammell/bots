cmake_minimum_required(VERSION 3.10)
project(robogirls)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_subdirectory(include)
add_subdirectory(src)

#add_subdirectory(data)

add_library(VNgine ${RAZOR_CORE_HEADERS} ${RAZOR_CORE_SRC})
add_executable(game ${RAZOR_GAME_HEADERS} ${RAZOR_GAME_SRC})
add_executable(test ${RAZOR_TEST_HEADERS} ${RAZOR_TEST_SRC})

if(MSVC)
  # Remove default CMake warning level
  string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

  # clang-cl interprets /W4 as -Weverything which is too much
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message("clang-cl detected")
    target_compile_options(VNgine PUBLIC
      /clang:-Wall
      /clang:-Wextra
    )
  else()
    target_compile_options(VNgine PUBLIC
      /W4
    )
  endif()

  if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(
      /WX
    )
  else()
  endif()

  # Disable C++ exceptions.
  string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  target_compile_definitions(VNgine PUBLIC
    _HAS_EXCEPTIONS=0
  )
  # Disable RTTI.
  string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  target_compile_options(VNgine PUBLIC
    /GR-
  )

else()
  add_compile_options(-Wall -Wextra -pedantic)
  if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-Werror)
  endif()

  # Disable C++ exceptions.
  string(REGEX REPLACE "-fexceptions" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  add_compile_options(-fno-exceptions)
  # Disable RTTI.
  string(REGEX REPLACE "-frtti" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  add_compile_options(-fno-rtti)
    
endif()

# GLFW
# Disable GLFW docs, tests and examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)
target_include_directories(VNgine PUBLIC lib/glfw/include)
target_link_libraries(VNgine glfw)

# GLEW
option (glew-cmake_BUILD_SHARED "" OFF)
option(glew-cmake_BUILD_MULTI_CONTEXT "" OFF)
add_subdirectory(lib/glew)
target_include_directories(VNgine PUBLIC lib/glew/include)
target_link_libraries(VNgine libglew_static)

# GLM
target_include_directories(VNgine PUBLIC lib/glm)

#Extend compiler/OS specific libs
if (WIN32)
  target_link_libraries(VNgine
    opengl32
    glu32
  )
else ()
  target_link_libraries(VNgine
    OpenGL
    GLX
    stdc++fs
  )
endif ()

target_link_libraries(game VNgine)

target_link_libraries(test VNgine)
