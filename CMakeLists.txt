cmake_minimum_required(VERSION 2.6)

PROJECT(MicroSpacecraft CXX)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

INCLUDE_DIRECTORIES(include import/include import/include/UniversumLib)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} import/lib)

FILE(GLOB SRC "src/*.cpp")
FILE(GLOB VIEW_SRC "src/view/*.cpp")
FILE(GLOB HEADER "include/*.h")
FILE(GLOB VIEW_HEADER "include/view/*.h")

SET(COMPLETE_SRC ${SRC} ${HEADER} ${VIEW_SRC} ${VIEW_HEADER})

if(MSVC)

# src
source_group("src" FILES ${SRC})
source_group("src//view" FILES ${VIEW_SRC})
# include
source_group("include" FILES ${HEADER})
source_group("include//view" FILES ${VIEW_HEADER})
#source_group("include" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/${REC2}" )
endif(MSVC)

#ENABLE_EXPORTS()
#DEFINE_SYMBOL(__TEST)


if(WIN32)
SET(PA import/lib)
find_library(SDL2_LIB SDL2 PATHS ${PA})
find_library(CORE2_LIB DRCore2 PATHS ${PA})
find_library(UNIVERSUM_LIB DRUniversumLib PATHS ${PA})
find_library(GLEW_32 glew32 PATHS ${PA})
SET(LIBS ${CORE2_LIB} ${UNIVERSUM_LIB} Winmm ${GLEW_32} opengl32 ${SDL2_LIB})
else(UNIX)
SET(LIBS DRCore2 DRUniversumLib GLEW)
endif(WIN32)

ADD_EXECUTABLE(MicroSpacecraft ${COMPLETE_SRC})
TARGET_LINK_LIBRARIES(MicroSpacecraft ${LIBS})

SET_TARGET_PROPERTIES(MicroSpacecraft PROPERTIES EXECUTABLE_OUTPUT_PATH bin)



