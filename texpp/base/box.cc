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

#include <texpp/base/box.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

namespace texpp {
namespace base {

bool BoxVariable::invokeOperation(Parser& parser,
                        shared_ptr<Node> node, Operation op, bool)
{
    // TODO: box should not derive from Variable!
    if(op == ASSIGN || op == GET) {
        string name = parseName(parser, node);
        Box box = parser.symbol(name, Box());
        node->setValue(box);
        return true;
    }
    return false;
}

bool Lastbox::invokeOperation(Parser& parser,
                        shared_ptr<Node> node, Operation op, bool global)
{
    if(parser.mode() == Parser::VERTICAL ||
                parser.mode() == Parser::MATH) {
        parser.logger()->log(Logger::ERROR,
            "You can't use `" + texRepr(&parser) + "' in " +
            parser.modeName() + " mode",
            parser, parser.lastToken());
        return true;
    }
    return BoxVariable::invokeOperation(parser, node, op, global);
}

string Vsplit::parseName(Parser& parser, shared_ptr<Node> node)
{
    string s = Register<BoxVariable>::parseName(parser, node);
    static vector<string> kw_to(1, "to");
    Node::ptr to = parser.parseKeyword(kw_to);
    if(!to) {
        parser.logger()->log(Logger::ERROR,
            "Missing `to' inserted",
            parser, parser.lastToken());
        to = Node::ptr(new Node("error_missing_to"));
        to->setValue(string("to"));
    }
    node->appendChild("to", to);
    node->appendChild("dimen", parser.parseDimen());
    //parser.setSymbolDefault(s, m_initValue);
    return s;
}

string Setbox::parseName(Parser& parser, shared_ptr<Node> node)
{
    shared_ptr<Node> number = parser.parseNumber();
    node->appendChild("variable_number", number);
    int n = number->value(int(0));

    if(n < 0 || n > 255) {
        parser.logger()->log(Logger::ERROR,
            "Bad register code (" + boost::lexical_cast<string>(n) + ")",
            parser, parser.lastToken());
        n = 0;
    }

    string s = this->name().substr(1) + boost::lexical_cast<string>(n);
    if(s.substr(0, 6) == "setbox")
        parser.setSymbolDefault(s.substr(3), m_initValue);
    return s;
}

bool Setbox::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());
        node->appendChild("filler", parser.parseFiller(true));

        Node::ptr rvalue =
            Variable::tryParseVariableValue<base::BoxVariable>(parser);
        if(!rvalue) {
            parser.logger()->log(Logger::ERROR,
                "A <box> was supposed to be here",
                parser, parser.lastToken());
            rvalue = Node::ptr(new Node("error_missing_box"));
            rvalue->setValue(Box());
        }

        node->appendChild("rvalue", rvalue);
        node->setValue(rvalue->valueAny());

        if(name.substr(0, 6) == "setbox")
            name = name.substr(3);
        parser.setSymbol(name, rvalue->valueAny(), global);
        return true;
    }
    return false;
}

bool BoxSpec::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool)
{
    if(op == ASSIGN || op == GET) {
        string name = parseName(parser, node);

        static vector<string> kw_spec;
        if(kw_spec.empty()) {
            kw_spec.push_back("to");
            kw_spec.push_back("spread");
        }

        Node::ptr spec = parser.parseOptionalKeyword(kw_spec);
        node->appendChild("spec_clause", spec);

        Dimen to(0);
        if(spec->value(string()) == "to") {
            Node::ptr toNode = parser.parseDimen();
            node->appendChild("to", toNode);
            to = toNode->value(Dimen(0));
        } else if(spec->value(string()) == "spread") {
            Node::ptr spreadNode = parser.parseDimen();
            node->appendChild("spread", spreadNode);
            to = spreadNode->value(Dimen(0));
        }

        node->appendChild("filler", parser.parseFiller(true));

        Parser::Mode prevMode = parser.mode();

        parser.beginGroup();
        parser.setMode(m_mode);
        parser.resetParagraphIndent();
        Node::ptr group = parser.parseGroup(Parser::GROUP_NORMAL);
        parser.setMode(prevMode);
        parser.endGroup();

        node->appendChild("content", group);

        Box box(m_mode, m_top);
        box.value = Token::list_ptr(new Token::list());

        if(m_mode == Parser::RHORIZONTAL) {
            box.height = to;
        } else if(m_mode == Parser::RVERTICAL) {
            if(m_top) box.skip = to;
            else box.width = to;
        }

        // TODO: fill token list!

        node->setValue(box);
        return true;

    }
    return false;
}

bool Rule::invoke(Parser& parser, shared_ptr<Node> node)
{
    static vector<string> kw_spec;
    if(kw_spec.empty()) {
        kw_spec.push_back("width");
        kw_spec.push_back("height");
        kw_spec.push_back("depth");
    }

    while(true) {
        Node::ptr spec = parser.parseOptionalKeyword(kw_spec);
        string name = spec->value(string());
        node->appendChild("spec_" + name, spec);
        if(name.empty()) break;

        Node::ptr dimen = parser.parseDimen();
        node->appendChild("dimen_" + name, dimen);
    }

    return true;
}

bool Rule::presetMode(Parser& parser)
{
    if(m_mode == Parser::VERTICAL &&
            parser.mode() != Parser::RHORIZONTAL) {
        parser.setMode(Parser::HORIZONTAL);
    } else if(m_mode == Parser::HORIZONTAL &&
            parser.mode() != Parser::RVERTICAL) {
        parser.setMode(Parser::VERTICAL);
    }
    return true;
}

} // namespace base
} // namespace texpp


