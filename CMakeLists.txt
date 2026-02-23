cmake_minimum_required(VERSION 3.16)

project(linuxUtilits VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES 
    core/Process/ProcessScanner.cpp core/Process/ProcessScanner.hpp
    core/Process/ProcessReader.cpp core/Process/ProcessReader.hpp
    core/Process/ProcessFinder.cpp core/Process/ProcessFinder.hpp
    core/Process/ModuleMapParser.cpp core/Process/ModuleMapParser.hpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})