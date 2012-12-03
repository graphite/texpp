set(HREFTEST_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/hreftest.py PARENT_SCOPE)
set(HREFKEYWORDS_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/hrefkeywords.py PARENT_SCOPE)

install(FILES latexstubs.py DESTINATION bin)
install(PROGRAMS hrefkeywords.py hrefliterals.py hreftest.py texpp.py DESTINATION bin)

set(_chrefliterals_SOURCES
    porter.c
    _chrefliterals.cc
)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${BOOST_PYTHON_INCLUDES})

add_library(_chrefliterals SHARED ${_chrefliterals_SOURCES})
target_link_libraries(_chrefliterals libtexpp ${BOOST_PYTHON_LIBS} ${Boost_FILESYSTEM_LIBRARY} ${Boost_REGEX_LIBRARY})
set_target_properties(_chrefliterals PROPERTIES PREFIX "" OUTPUT_NAME "_chrefliterals")

install(TARGETS _chrefliterals LIBRARY DESTINATION bin)

