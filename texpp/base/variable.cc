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

#include <texpp/base/variable.h>
#include <texpp/parser.h>
#include <texpp/logger.h>
#include <texpp/lexer.h>

#include <boost/lexical_cast.hpp>

namespace texpp {
namespace base {

string Variable::parseName(Parser&, shared_ptr<Node>)
{
    return name().substr(1);
}

bool Variable::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool)
{
    if(op == GET) {
        string name = parseName(parser, node);
        const any& ret = parser.symbolAny(name);
        node->setValue(ret.empty() ? m_initValue : ret);
        return true;
    }
    return false;
}

bool Variable::invokeWithPrefixes(Parser& parser, shared_ptr<Node> node,
                                    std::set<string>& prefixes)
{
    bool global = checkPrefixes(parser, prefixes);
    prefixes.clear();
    return invokeOperation(parser, node, ASSIGN, global);
}

bool ArithmeticCommand::invokeWithPrefixes(Parser& parser,
            shared_ptr<Node> node, std::set<string>& prefixes)
{
    bool global = checkPrefixes(parser, prefixes);
    prefixes.clear();

    Node::ptr lvalue = parser.parseToken();
    Token::ptr token = lvalue->value(Token::ptr());

    bool ok = false;
    shared_ptr<Variable> var = parser.symbolCommand<Variable>(token);
    if(var) {
        node->appendChild("lvalue", lvalue);
        ok = var->invokeOperation(parser, node, m_op, global);
        if(!ok) node->children().pop_back();
    }

    if(!ok) {
        string tname;
        Command::ptr cmd = parser.symbol(token, Command::ptr());
        if(cmd) tname = cmd->texRepr(&parser);
        else tname = token->meaning(&parser);
        parser.logger()->log(Logger::ERROR,
            string("You can't use `") + tname +
            string("' after ") + texRepr(&parser),
            parser, token);
        node->setValue(int(0));
        node->appendChild("error_wrong_lvalue", lvalue);
    }

    return ok;
}

} // namespace base
} // namespace texpp

