include_directories(${Boost_INCLUDE_DIRS})
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

add_library(libtexpp SHARED ${libtexpp_SOURCES})
set_target_properties(libtexpp PROPERTIES OUTPUT_NAME texpp)

# Temporary hack
install(TARGETS libtexpp LIBRARY DESTINATION bin)

