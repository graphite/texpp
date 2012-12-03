cmake_minimum_required(VERSION 2.6)
project(texpp)

# Enable testing
include(CTest)

# Find boost libraries
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.34.0 COMPONENTS filesystem regex python REQUIRED)

# Find python interpreter
find_package(PythonInterp)
if(NOT PYTHONINTERP_FOUND)
    message("python interpreter NOT found - tests/hrefkeywords will NOT be built.")
endif(NOT PYTHONINTERP_FOUND)

# Find boost.python libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})
find_package(BoostPython)
if(NOT BOOST_PYTHON_FOUND)
    message("boost.python NOT found - texpy and hrefkeywords will NOT be built.")
endif(NOT BOOST_PYTHON_FOUND)

# Find tex executable (required for tests/tex and tests/hrefkeywords)
find_program(TEX_EXECUTABLE "tex")
if(TEX_EXECUTABLE STREQUAL "TEX_EXECUTABLE-NOTFOUND")
    message("tex executable NOT found - tests/tex and tests/hrefkeywords will NOT be built.")
else(TEX_EXECUTABLE STREQUAL "TEX_EXECUTABLE-NOTFOUND")
    set(TEX_FOUND ON)
endif(TEX_EXECUTABLE STREQUAL "TEX_EXECUTABLE-NOTFOUND")
mark_as_advanced(TEX_EXECUTABLE)

# Flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wunused -Woverloaded-virtual -Wwrite-strings")
include_directories(${CMAKE_SOURCE_DIR})

# Subdirectories
add_subdirectory(texpp)

if(BOOST_PYTHON_FOUND)
    add_subdirectory(texpy)
    add_subdirectory(hrefkeywords)
endif(BOOST_PYTHON_FOUND)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif(BUILD_TESTING)

