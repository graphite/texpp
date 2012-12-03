/*  This file is part of texpp library.
    Copyright (C) 2009 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <boost/python.hpp>
#include <texpp/token.h>
#include <texpp/parser.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

void export_token_class()
{
    using namespace boost::python;
    using namespace texpp;

    scope scope_Token = class_<Token, shared_ptr<Token> >(
            "Token", init<Token::Type, Token::CatCode, const string&,
                    const string&, size_t, size_t, size_t>())
        .def(init<Token::Type, Token::CatCode,
                    const string&, const string&, size_t, size_t>())
        .def(init<Token::Type, Token::CatCode, const string&,
                    const string&, size_t>())
        .def(init<Token::Type, Token::CatCode, const string&,
                    const string&>())
        .def(init<Token::Type, Token::CatCode, const string&>())

        .def("__repr__", &Token::repr)

        .add_property("type", &Token::type, &Token::setType)
        .add_property("catCode", &Token::catCode, &Token::setCatCode)
        .add_property("value", make_function(&Token::value,
                    return_value_policy<copy_const_reference>()),
                    &Token::setValue)
        .add_property("source", make_function(&Token::source,
                    return_value_policy<copy_const_reference>()),
                    &Token::setSource)
        .add_property("linePos", &Token::lineNo, &Token::setLinePos)
        .add_property("lineNo", &Token::lineNo, &Token::setLineNo)
        .add_property("charPos", &Token::charPos, &Token::setCharPos)
        .add_property("charEnd", &Token::charEnd, &Token::setCharEnd)
        .def("fileName", &Token::fileName,
                    return_value_policy<copy_const_reference>())
        .def("isSkipped", &Token::isSkipped)
        .def("isControl", &Token::isControl)
        .def("isCharacter", (bool (Token::*)() const) &Token::isSkipped)
        .def("isCharacter", (bool (Token::*)(char) const) &Token::isCharacter)
        .def("isCharacter", (bool (Token::*)(char, Token::CatCode) const)
                                    &Token::isCharacter)
        .def("isCharacterCat", (bool (Token::*)(Token::CatCode) const)
                                    &Token::isCharacterCat)
        .def("isLastInLine", &Token::isLastInLine)
        .def("texRepr", &Token::texRepr)
        .def("meaning", &Token::meaning)
        ;

    scope_Token.attr("npos") = size_t(Token::npos);

    enum_<Token::Type>("Type")
        .value("SKIPPED", Token::TOK_SKIPPED)
        .value("CHARACTER", Token::TOK_CHARACTER)
        .value("CONTROL", Token::TOK_CONTROL)
        ;

    enum_<Token::CatCode>("CatCode")
        .value("ESCAPE", Token::CC_ESCAPE)
        .value("BGROUP", Token::CC_BGROUP)
        .value("EGROUP", Token::CC_EGROUP)
        .value("MATHSHIFT", Token::CC_MATHSHIFT)
        .value("ALIGNTAB", Token::CC_ALIGNTAB)
        .value("EOL", Token::CC_EOL)
        .value("PARAM", Token::CC_PARAM)
        .value("SUPER", Token::CC_SUPER)
        .value("SUB", Token::CC_SUB)
        .value("IGNORED", Token::CC_IGNORED)
        .value("SPACE", Token::CC_SPACE)
        .value("LETTER", Token::CC_LETTER)
        .value("OTHER", Token::CC_OTHER)
        .value("ACTIVE", Token::CC_ACTIVE)
        .value("COMMENT", Token::CC_COMMENT)
        .value("INVALID", Token::CC_INVALID)
        .value("NONE", Token::CC_NONE)
        ;
}

void export_token()
{
    using namespace boost::python;
    using namespace texpp;
    export_token_class();

    class_<std::vector< shared_ptr<Token> > >("TokenList")
        .def(vector_indexing_suite<std::vector< shared_ptr<Token> >, true >())
    ;
}

