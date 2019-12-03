cmake_minimum_required(VERSION 2.8.11)
# Name of the project
set(PROJECT_NAME VertexArrayObjects	)
project(${PROJECT_NAME})

add_subdirectory(${PROJECT_SOURCE_DIR}/Boid/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/BoidShaded/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/SimpleCube/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/VAOSphere/ )
