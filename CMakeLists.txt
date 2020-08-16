cmake_minimum_required(VERSION 3.10)
project(robogirls)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

#add_subdirectory(lib)
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

target_include_directories(VNgine PUBLIC ${RAZOR_CORE_LIB_INCLUDES})
target_link_libraries(VNgine PUBLIC ${RAZOR_CORE_LIBRARIES})

target_link_libraries(game VNgine)

target_link_libraries(test VNgine)

