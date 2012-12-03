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

#include <texpp/base/toks.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

namespace texpp {
namespace base {

bool InternalToks::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());

        Node::ptr internal =
            Variable::tryParseVariableValue<base::InternalToks>(parser);
        if(internal) {
            node->setValue(internal->valueAny());
        } else {
            internal = parser.parseGeneralText(false);
            Token::list_ptr tokens = internal->child("balanced_text")
                                        ->value(Token::list_ptr());
            node->setValue(tokens ? *tokens : Token::list());
        }
        node->appendChild("rvalue", internal);
        parser.setSymbol(name, node->valueAny(), global);
        return true;

    } else if(op == EXPAND) {
        string name = parseName(parser, node);
        Token::list toks = parser.symbol(name, Token::list());
        node->setValue(toksToString(parser, toks));
        return true;

    }
    return Variable::invokeOperation(parser, node, op, global);
}

string InternalToks::toksToString(Parser& parser, const Token::list& toks)
{
    return Token::texReprList(toks, &parser);
    /*
    char newlinechar = parser.symbol("newlinechar", int(0));
    char escapechar = parser.symbol("escapechar", int(0));
    BOOST_FOREACH(Token::ptr token, toks) {
        if(token->isControl()) {
            str += token->texRepr(escapechar);
            if(token->value().size() > 1 &&
                    token->value()[0] == '\\') {
                 int ccode = parser.symbol("catcode" +
                    boost::lexical_cast<string>(int(token->value()[1])),
                    int(0));
                if(ccode == Token::CC_LETTER) str += ' ';
            }
        } else if(token->isCharacter(newlinechar)) {
            str += '\n';
        } else if(token->isCharacter()) {
            str += token->value();
        }
    }
    return str;
    */
}

} // namespace base
} // namespace texpp

