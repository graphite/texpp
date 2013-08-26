include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIRS})

set(libtexpy_SOURCES
    python_file_stream.cc
    boost_any.cc
    std_set.cc
    token.cc
    lexer.cc
    command.cc
    parser.cc
    logger.cc
    texpy.cc
)

set(libtexpy_HEADERS
    python_file_stream.h
    shared_ptr.h
    std_pair.h
)

add_definitions(-fPIC)

add_library(libtexpy SHARED ${libtexpy_SOURCES} ${libtexpy_HEADERS})
target_link_libraries(libtexpy libtexpp ${Boost_PYTHON_LIBRARY} ${PYTHON_LIBRARIES})
set_target_properties(libtexpy PROPERTIES PREFIX "" SUFFIX ".so" OUTPUT_NAME texpy)

set(TEXPY_SO_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

install(TARGETS libtexpy LIBRARY DESTINATION bin)

