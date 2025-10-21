cmake_minimum_required(VERSION 3.16)

project(XFileUnpacker
    VERSION 0.1.0
    DESCRIPTION "Generic Qt5/Qt6 project with Console/GUI versions"
    LANGUAGES CXX
)

# Set CMake policy
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# Find Qt5 or Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automoc, autorcc, autouic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add subdirectories
add_subdirectory(src)

# Enable testing
enable_testing()
add_subdirectory(test)

# Installation targets
install(FILES README.md DESTINATION .)
install(FILES LICENSE.txt DESTINATION .)
install(FILES changelog.txt DESTINATION .)
install(DIRECTORY doc/ DESTINATION doc)
install(DIRECTORY res/ DESTINATION res)
