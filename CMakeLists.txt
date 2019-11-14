cmake_minimum_required(VERSION 3.0.0)
project(pc_gb VERSION 0.1.0)

include(CTest)
enable_testing()

if(WIN32)
    include_directories(thirdparty/SDL2.0-mingw32/include)
    link_directories(thirdparty/SDL2.0-mingw32/lib)
endif (WIN32)

if(APPLE)
    include_directories(/Library/Frameworks/SDL2.framework/Headers)
    find_library(SDL2_LIB SDL2)
endif(APPLE)

add_executable(pc_gb main.c)

if(WIN32)
    target_link_libraries(pc_gb mingw32 SDL2main SDL2)
endif(WIN32)

if(APPLE)
    target_link_libraries(pc_gb ${SDL2_LIB})
endif(APPLE)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
