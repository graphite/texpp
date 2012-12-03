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
#include <texpp/parser.h>

#include <boost/any.hpp>
#include <memory>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>

#include "std_pair.h"
#include "shared_ptr.h"

/*
namespace texpp { namespace {

using boost::python::object;

class ParserWrap: public Parser {
public:
    ParserWrap(const string& fileName, const std::auto_ptr<std::istream>& file,
            bool interactive = false,
            shared_ptr<Logger> logger = shared_ptr<Logger>())
        : Parser(fileName, const_cast<std::auto_ptr<std::istream>&>(file),
                    interactive, logger) {}

    const any& symbol0(const string& name) const {
        return Parser::symbolAny(name);
    }
    const any& symbol1(Token::ptr token) const {
        return Parser::symbolAny(token);
    }
    void setSymbol0(const string& name, const any& value) {
        Parser::setSymbol(name, value);
    }
    void setSymbol1(Token::ptr token, const any& value) {
        Parser::setSymbol(token, value);
    }

    bool invokeCommand(Command::ptr cmd, Node::ptr node) {
        return cmd->invoke(*this, node);
    }
};

}}*/

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(
    Node_treeRepr_overloads, treeRepr, 0, 1)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(
    Node_source_overloads, source, 0, 1)

void export_node()
{
    using namespace boost::python;
    using namespace texpp;

    export_std_pair<string, Node::ptr>();
    export_std_pair<size_t, size_t>();
    export_shared_ptr<string>();

    scope scopeNode = class_<Node, shared_ptr<Node> >(
            "Node", init<std::string>())
        .def("__repr__", &Node::repr)
        .def("treeRepr", &Node::treeRepr,
            Node_treeRepr_overloads())
        
        .def("source", &Node::source,
            Node_source_overloads())
        .def("sources", &Node::sources)
        .def("files", &Node::files)
        .def("oneFile", &Node::oneFile)
        .def("isOneFile", &Node::isOneFile)
        .def("sourcePos", &Node::sourcePos)

        .def("setType", &Node::setType)
        .def("type", &Node::type,
            return_value_policy<copy_const_reference>())
        .def("setValue", &Node::setValue)
        .def("value", &Node::valueAny,
            return_value_policy<return_by_value>())
        .def("tokens", (vector<Token::ptr>& (Node::*)())(&Node::tokens),
            return_internal_reference<1,
                return_value_policy<reference_existing_object> >())
        .def("childrenCount", &Node::childrenCount)
        .def("children", (Node::ChildrenList& (Node::*)())(&Node::children),
            return_internal_reference<1,
                return_value_policy<reference_existing_object> >())
        .def("child", (Node::ptr (Node::*)(const string&))(&Node::child))
        .def("child", (Node::ptr (Node::*)(int))(&Node::child))
        .def("appendChild", &Node::appendChild)
        ;

    class_< std::vector<size_t> >("SizeTVector")
        .def(vector_indexing_suite< std::vector<size_t> >())
    ;

    class_<Node::ChildrenList>("ChildrenList")
        .def(vector_indexing_suite<Node::ChildrenList, true >())
    ;

    class_< unordered_map<shared_ptr<string>, string> >("SourcesMap")
        .def(map_indexing_suite< unordered_map<shared_ptr<string>, string>, true >())
    ;
}

#define PARSER_OVERLOADS(name, n1, n2) \
    BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( \
            Parser_##name##_overloads, name, n1, n2)

PARSER_OVERLOADS(setSymbol, 2, 3)

PARSER_OVERLOADS(peekToken, 0, 1)
PARSER_OVERLOADS(nextToken, 0, 2)

PARSER_OVERLOADS(parseToken, 0, 1)
PARSER_OVERLOADS(parseGeneralText, 1, 2)
PARSER_OVERLOADS(parseControlSequence, 0, 1)

void export_parser()
{
    using namespace boost::python;
    using namespace texpp;
    using boost::any;

    export_node();

    scope scopeParser = class_<Parser, boost::noncopyable >("Parser",
            init<std::string, shared_ptr<std::istream>,
                 std::string, bool, bool, shared_ptr<Logger> >())
        .def(init<std::string, shared_ptr<std::istream>, std::string, bool, bool>())
        .def(init<std::string, shared_ptr<std::istream>, std::string, bool>())
        .def(init<std::string, shared_ptr<std::istream>, std::string >())
        .def(init<std::string, shared_ptr<std::istream> >())

        .def("parse", &Parser::parse)

        .def("workdir", &Parser::workdir,
            return_value_policy<copy_const_reference>())
        .def("setWorkdir", &Parser::setWorkdir)

        // Tokens
        .def("peekToken", &Parser::peekToken,
            Parser_peekToken_overloads())
        .def("nextToken", &Parser::nextToken,
            Parser_nextToken_overloads())
        .def("lastToken", &Parser::lastToken)

        // parse*
        .def("parseToken", &Parser::parseToken,
            Parser_parseToken_overloads())
        .def("parseGroup", &Parser::parseGroup)
        .def("parseGeneralText", &Parser::parseGeneralText,
            Parser_parseGeneralText_overloads())
        .def("parseControlSequence", &Parser::parseControlSequence,
            Parser_parseControlSequence_overloads())

        .def("parseOptionalSpaces", &Parser::parseOptionalSpaces)
        .def("parseFileName", &Parser::parseFileName)

        // Symbols
        .def("symbol", (const any& (Parser::*)(const string&) const)(
                        &Parser::symbolAny),
                        return_value_policy<return_by_value>())
        .def("symbol", (const any& (Parser::*)(Token::ptr) const)(
                        &Parser::symbolAny),
                        return_value_policy<return_by_value>())
        .def("setSymbol", (void (Parser::*)(const string&, const any&, bool))
                        (&Parser::setSymbol),
                        Parser_setSymbol_overloads())
        .def("setSymbol", (void (Parser::*)(Token::ptr, const any&, bool))
                        (&Parser::setSymbol),
                        Parser_setSymbol_overloads())
        //.def("setSymbol", &ParserWrap::setSymbol0)
        //.def("setSymbol", &ParserWrap::setSymbol1)
        .def("beginGroup", &Parser::beginGroup)
        .def("endGroup", &Parser::endGroup)
        .def("beginCustomGroup", &Parser::beginCustomGroup)
        .def("endCustomGroup", &Parser::endCustomGroup)

        .def("input", &Parser::input)

        .def("end", &Parser::end)
        ;

    enum_<Parser::Mode>("Mode")
        .value("VERTICAL", Parser::VERTICAL)
        .value("RVERTICAL", Parser::RVERTICAL)
        .value("HORIZONTAL", Parser::HORIZONTAL)
        .value("RHORIZONTAL", Parser::RHORIZONTAL)
        .value("MATH", Parser::MATH)
        ;

    enum_<Parser::GroupType>("GroupType")
        .value("NORMAL", Parser::GROUP_NORMAL)
        .value("MATH", Parser::GROUP_MATH)
        .value("CUSTOM", Parser::GROUP_CUSTOM)
        ;

}

