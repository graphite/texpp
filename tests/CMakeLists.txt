add_executable(texpp texpp.cc)
target_link_libraries(texpp libtexpp)
set(TEXPP_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/texpp)

add_executable(test_lexer test_lexer.cc)
target_link_libraries(test_lexer libtexpp)
add_test(test_lexer ${EXECUTABLE_OUTPUT_PATH}/test_lexer)

add_executable(test_parser test_parser.cc)
target_link_libraries(test_parser libtexpp)
add_test(test_parser ${EXECUTABLE_OUTPUT_PATH}/test_parser)

if(TEX_FOUND)
    add_subdirectory(tex)
endif(TEX_FOUND)

if(BOOST_PYTHON_FOUND AND PYTHONINTERP_FOUND AND TEX_FOUND)
    add_subdirectory(hrefkeywords)
endif(BOOST_PYTHON_FOUND AND PYTHONINTERP_FOUND AND TEX_FOUND)

