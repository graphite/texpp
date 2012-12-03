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

#include <texpp/base/dimen.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <sstream>

namespace texpp {
namespace base {

bool InternalDimen::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());
        Node::ptr rvalue = parser.parseDimen();
        node->appendChild("rvalue", rvalue);

        node->setValue(rvalue->valueAny());
        parser.setSymbol(name, rvalue->valueAny(), global);
        return true;
    } else if(op == EXPAND) {
        string name = parseName(parser, node);
        Dimen val = parser.symbol(name, Dimen(0));
        node->setValue(dimenToString(val));
        return true;
    } else {
        return Variable::invokeOperation(parser, node, op, global);
    }
}

bool DimenVariable::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    static vector<string> kw_by(1, "by");
    if(op == ADVANCE) {
        string name = parseName(parser, node);
        
        node->appendChild("by", parser.parseOptionalKeyword(kw_by));

        Node::ptr rvalue = parser.parseDimen();
        node->appendChild("rvalue", rvalue);

        Dimen v = parser.symbol(name, Dimen(0));
        v.value += rvalue->value(Dimen(0)).value;

        node->setValue(v);
        parser.setSymbol(name, v, global);
        return true;

    } else if(op == MULTIPLY || op == DIVIDE) {
        string name = parseName(parser, node);

        node->appendChild("by", parser.parseOptionalKeyword(kw_by));

        Node::ptr rvalue = parser.parseNumber();
        node->appendChild("rvalue", rvalue);

        Dimen v = parser.symbol(name, Dimen(0));
        int rv = rvalue->value(int(0));
        bool overflow = false;

        if(op == MULTIPLY) {
            pair<int,bool> p = safeMultiply(v.value, rv,  TEXPP_SCALED_MAX);
            v.value = p.first; overflow = p.second;
        } else if(op == DIVIDE) {
            pair<int,bool> p = safeDivide(v.value, rv);
            v.value = p.first; overflow = p.second;
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

    return InternalDimen::invokeOperation(parser, node, op, global);
}

tuple<int,int,bool> InternalDimen::multiplyIntFrac(int x, int n, int d)
{
    int sign = 1;
    if(x < 0) { x=-x; sign=-1; }
    unsigned int t = (x % 0x8000) * n;
    unsigned int u = (x / 0x8000) * n + (t / 0x8000);
    unsigned int v = (u % d) * 0x8000 + (t % 0x8000);
    if(u / d >= 0x8000) // XXX: error
        return boost::make_tuple(0, 0, true);
    return boost::make_tuple(sign * int((u / d) * 0x8000 + (v / d)),
                    sign * (v % d), false);
}

string InternalDimen::dimenToString(Dimen nn, int o, bool mu)
{
    std::ostringstream s;
    int n = nn.value;
    if(n<0) { s << '-'; n=-n; }

    s << n/0x10000 << '.';

    n = 10*(n & 0xffff) + 5;
    int delta = 10;
    do {
        if(delta > 0x10000) n+=0x8000-50000;
        s << char('0' + n/0x10000);
        n = 10*(n & 0xffff);
        delta *= 10;
    } while(n > delta);

    if(o == 0 && !mu) s << "pt";
    else if(o == 0 && mu) s << "mu";
    else if(o == 1) s << "fil";
    else if(o == 2) s << "fill";
    else if(o == 3) s << "filll";
    return s.str();
}

bool SpecialDimen::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());
        Node::ptr rvalue = parser.parseDimen();
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
        Dimen val = parser.symbol(name, Dimen(0));
        node->setValue(dimenToString(val));
        return true;
    } else {
        return InternalDimen::invokeOperation(parser, node, op, global);
    }
}

string BoxDimen::parseName(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("variable_number", number);
    int n = number->value(int(0));

    if(n < 0 || n > 255) {
        parser.logger()->log(Logger::ERROR,
            "Bad register code (" + boost::lexical_cast<string>(n) + ")",
            parser, parser.lastToken());
        n = 0;
    }

    string s = name().substr(1) + boost::lexical_cast<string>(n);
    parser.setSymbolDefault(s, m_initValue);
    return s;
}

bool BoxDimen::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());
        Node::ptr rvalue = parser.parseDimen();
        node->appendChild("rvalue", rvalue);

        node->setValue(rvalue->valueAny());
        return true;
    } else {
        return InternalDimen::invokeOperation(parser, node, op, global);
    }
}

} // namespace base
} // namespace texpp

