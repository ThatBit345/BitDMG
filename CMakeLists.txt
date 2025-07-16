cmake_minimum_required(VERSION 3.16)
project(bitboy)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

include_directories(${PROJECT_SOURCE_DIR}/include
					${PROJECT_SOURCE_DIR}/src)
		
file(GLOB SRCS 
		"${PROJECT_SOURCE_DIR}/include/*h"
		"${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(BitBoy ${SRCS})
target_link_libraries(BitBoy PRIVATE SDL3::SDL3)