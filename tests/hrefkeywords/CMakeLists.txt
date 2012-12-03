set(ARXIV_FILES_DIR "" CACHE FILEPATH "Path to test data from arxiv.org")

if(PYTHONINTERP_FOUND)
add_test(test_hrefliterals ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hrefliterals.py ${CMAKE_SOURCE_DIR}/hrefkeywords ${CMAKE_BINARY_DIR}/texpy ${CMAKE_BINARY_DIR}/hrefkeywords)
endif(PYTHONINTERP_FOUND)

if(ARXIV_FILES_DIR)

file(GLOB ARXIV_TEST_FILES "${ARXIV_FILES_DIR}/*")
foreach(_arxiv_file ${ARXIV_TEST_FILES})
    get_filename_component(_arxiv_file_name ${_arxiv_file} NAME)

    add_test(test_hreftest_${_arxiv_file_name}
         ${PYTHON_EXECUTABLE} ${HREFTEST_EXECUTABLE} -t ${ARXIV_FILES_DIR} -f ${_arxiv_file_name}
         -c "PYTHONPATH=${TEXPY_SO_DIR} ${PYTHON_EXECUTABLE} ${HREFKEYWORDS_EXECUTABLE} -c ${CMAKE_CURRENT_SOURCE_DIR}/concepts4.txt -s -m %(macro)s -o %(output)s %(source)s"
         )
    set_property(TEST test_hreftest_${_arxiv_file_name} PROPERTY TIMEOUT 100)
endforeach(_arxiv_file ${ARXIV_TEST_FILES})

endif(ARXIV_FILES_DIR)

