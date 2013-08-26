include_directories(${Boost_INCLUDE_DIR})
#add_definitions(-DBOOST_NO_EXCEPTIONS)

add_definitions(-fPIC)

set(libtexpp_SOURCES
    common.cc
    token.cc
    lexer.cc
    logger.cc
    parser.cc
    command.cc
    kpsewhich.cc
    base/conditional.cc
    base/miscmacros.cc
    base/misc.cc
    base/show.cc
    base/func.cc
    base/files.cc
    base/variable.cc
    base/integer.cc
    base/dimen.cc
    base/glue.cc
    base/toks.cc
    base/font.cc
    base/char.cc
    base/parshape.cc
    base/hyphenation.cc
    base/box.cc
    base/base.cc
)

set(libtexpp_HEADERS
    common.h
    token.h
    logger.h
    lexer.h
    parser.h
    command.h
    kpsewhich.h
    base/conditional.h
    base/miscmacros.h
    base/misc.h
    base/show.h
    base/func.h
    base/files.h
    base/variable.h
    base/integer.h
    base/dimen.h
    base/glue.h
    base/toks.h
    base/font.h
    base/char.h
    base/parshape.h
    base/hyphenation.h
    base/box.h
    base/base.h
)

add_library(libtexpp STATIC ${libtexpp_SOURCES} ${libtexpp_HEADERS})
target_link_libraries(libtexpp ${Boost_LIBRARIES})
set_target_properties(libtexpp PROPERTIES OUTPUT_NAME texpp)

# Temporary hack
#install(TARGETS libtexpp LIBRARY DESTINATION bin)

