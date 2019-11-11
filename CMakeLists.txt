cmake_minimum_required(VERSION 3.0.0)
project(pc_gb VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(thirdparty/SDL2.0-mingw32/include)
link_directories(thirdparty/SDL2.0-mingw32/lib)

add_executable(pc_gb main.c)

target_link_libraries(pc_gb mingw32 SDL2main SDL2)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
