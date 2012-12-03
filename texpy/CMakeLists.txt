include_directories(${Boost_INCLUDE_DIRS})
include_directories(${BOOST_PYTHON_INCLUDES})

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

add_definitions(-fPIC)

add_library(libtexpy SHARED ${libtexpy_SOURCES})
target_link_libraries(libtexpy libtexpp ${BOOST_PYTHON_LIBS})
set_target_properties(libtexpy PROPERTIES PREFIX "" OUTPUT_NAME texpy)

set(TEXPY_SO_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

install(TARGETS libtexpy LIBRARY DESTINATION bin)

