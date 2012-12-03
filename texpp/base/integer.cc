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

#include <texpp/base/integer.h>
#include <texpp/parser.h>
#include <texpp/logger.h>
#include <texpp/lexer.h>

#include <boost/lexical_cast.hpp>

namespace texpp {
namespace base {

bool InternalInteger::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());
        Node::ptr rvalue = parser.parseNumber();
        node->appendChild("rvalue", rvalue);

        node->setValue(rvalue->valueAny());
        parser.setSymbol(name, rvalue->valueAny(), global);
        return true;
    } else if(op == Variable::EXPAND) {
        string name = parseName(parser, node);
        int val = parser.symbol(name, int(0));
        node->setValue(boost::lexical_cast<string>(val));
        return true;
    } else {
        return Variable::invokeOperation(parser, node, op, global);
    }
}

bool IntegerVariable::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ADVANCE || op == MULTIPLY || op == DIVIDE) {
        string name = parseName(parser, node);

        static vector<string> kw_by(1, "by");
        node->appendChild("by", parser.parseOptionalKeyword(kw_by));

        Node::ptr rvalue = parser.parseNumber();
        node->appendChild("rvalue", rvalue);

        int v = parser.symbol(name, int(0));
        int rv = rvalue->value(int(0));
        bool overflow = false;

        if(op == ADVANCE) {
            v += rv;
        } else if(op == MULTIPLY) {
            pair<int,bool> p = safeMultiply(v, rv,  TEXPP_INT_MAX);
            v = p.first; overflow = p.second;
        } else if(op == DIVIDE) {
            pair<int,bool> p = safeDivide(v, rv);
            v = p.first; overflow = p.second;
        }

        if(overflow) {
            parser.logger()->log(Logger::ERROR,
                "Arithmetic overflow",
                parser, parser.lastToken());
        } else {
            node->setValue(v);
            parser.setSymbol(name, v, global);
        }
        return true;
    }

    return InternalInteger::invokeOperation(parser, node, op, global);
}

string CharcodeVariable::parseName(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("variable_number", number);
    int n = number->value(int(0));

    if(n < 0 || n > 255) {
        parser.logger()->log(Logger::ERROR,
            "Bad character code (" + boost::lexical_cast<string>(n) + ")",
            parser, parser.lastToken());
        n = 0;
    }

    string s = name().substr(1) + boost::lexical_cast<string>(n);
    parser.setSymbolDefault(s, m_initValue);
    return s;
}

bool CharcodeVariable::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());
        Node::ptr rvalue = parser.parseNumber();
        node->appendChild("rvalue", rvalue);

        int n = rvalue->value(int(0));
        if(n < m_min || n > m_max) {
            parser.logger()->log(Logger::ERROR, "Invalid code (" +
                boost::lexical_cast<string>(n) +
                "), should be in the range " +
                boost::lexical_cast<string>(m_min) + ".." +
                boost::lexical_cast<string>(m_max),
                parser, parser.lastToken());
            n = 0;
        }

        node->setValue(n);
        parser.setSymbol(name, n, global);
        return true;
    } else {
        return InternalInteger::invokeOperation(parser, node, op, global);
    }
}

bool SpecialInteger::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());
        Node::ptr rvalue = parser.parseNumber();
        node->appendChild("rvalue", rvalue);

        node->setValue(rvalue->valueAny());
        parser.setSymbol(name, rvalue->valueAny(), true); // global
        return true;
    } else if(op == GET) {
        string name = parseName(parser, node);
        const any& ret = parser.symbolAny(name);
        node->setValue(ret.empty() ? m_initValue : ret);
        return true;
    } else if(op == EXPAND) {
        string name = parseName(parser, node);
        int val = parser.symbol(name, int(0));
        node->setValue(boost::lexical_cast<string>(val));
        return true;
    } else {
        return InternalInteger::invokeOperation(parser, node, op, global);
    }
}

bool Spacefactor::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(parser.mode() != Parser::HORIZONTAL &&
            parser.mode() != Parser::RHORIZONTAL) {
        parser.logger()->log(Logger::ERROR,
            op == GET || op == EXPAND ? "Improper " + texRepr(&parser) :
            "You can't use `" + texRepr(&parser) +
            "' in " + parser.modeName() + " mode",
            parser, parser.lastToken());
        if(op == GET) node->setValue(int(0));
        else if(op == EXPAND) node->setValue(string("0"));
        return true;
    }

    return SpecialInteger::invokeOperation(parser, node, op, global);
}

} // namespace base
} // namespace texpp

