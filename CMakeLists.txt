cmake_minimum_required(VERSION 3.0.0)
project(generate_sdf_image VERSION 0.1.0)

include(CTest)
enable_testing()

set(APP_SOURCES ./src/main.cpp)

include_directories("./stb" "./args")
add_executable(generate_sdf_image ${APP_SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
