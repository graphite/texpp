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

#include <texpp/base/show.h>
#include <texpp/base/integer.h>
#include <texpp/base/dimen.h>
#include <texpp/base/glue.h>
#include <texpp/base/toks.h>
#include <texpp/base/font.h>
#include <texpp/base/func.h>
#include <texpp/base/files.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace {

using namespace texpp;

bool parseMeaning(Parser& parser, shared_ptr<Node> node, bool show)
{
    Node::ptr tokenNode = parser.parseToken(false);
    Token::ptr token = tokenNode->value(Token::ptr());
    node->appendChild("token", tokenNode);

    string str;
    if(token->isCharacter()) {
        str = token->meaning(&parser);
    } else {
        if(show)
            str = token->texRepr(&parser) + '=';

        Command::ptr c = parser.symbol(token, Command::ptr());
        if(c) str += c->texRepr(&parser);
        else str += "undefined";

        // XXX: the following line
        if(!show && dynamic_pointer_cast<base::UserMacro>(c))
            boost::algorithm::erase_all(str, "\n");
    }

    if(show)
        parser.logger()->log(Logger::SHOW, str, parser, parser.lastToken());
    else
        node->setValue(Macro::stringToTokens(str));

    return true;
}

bool parseThe(Parser& parser, shared_ptr<Node> node, bool show)
{
    Node::ptr tokenNode = parser.parseToken();
    Token::ptr token = tokenNode->value(Token::ptr());
    node->appendChild("token", tokenNode);

    string str;
    shared_ptr<base::Variable> var =
        parser.symbolCommand<base::Variable>(token);

    if(!show && dynamic_pointer_cast<base::ToksVariable>(var)) {
        var->invokeOperation(parser, node, base::Variable::GET, false);
        Token::list toks = node->value(Token::list());
        Token::list_ptr toks_copy(new Token::list(toks.size()));
        for(size_t n=0; n<toks.size(); ++n) {
            (*toks_copy)[n] = toks[n]->lcopy();
            if(!show) {
                Command::ptr c = parser.prevCommand();
                if(dynamic_pointer_cast<base::Write>(c) ||
                        dynamic_pointer_cast<base::Message>(c) ||
                        (dynamic_pointer_cast<base::Def>(c) &&
                         static_pointer_cast<base::Def>(c)->expand())) {
                    parser.addNoexpand((*toks_copy)[n]);
                }
            }
        }
        node->setValue(toks_copy);
        return true;

    } else if(var) {
        bool ok = var->invokeOperation(parser, node,
                            base::Variable::EXPAND, false);
        if(ok) str = node->value(string());

    } else {
        string tname = token->texRepr(&parser);
        Command::ptr cmd = parser.symbol(token, Command::ptr());
        if(cmd) tname = cmd->texRepr(&parser);
        parser.logger()->log(Logger::ERROR,
            "You can't use `" + tname +
            "' after " + parser.escapestr() + "the",
            parser, token);
        str = "0";
    }

    if(show)
        parser.logger()->log(Logger::SHOW, str,
            parser, parser.lastToken());
    else
        node->setValue(Macro::stringToTokens(str));

    return true;
}

} // namespace

namespace texpp {
namespace base {

bool Show::invoke(Parser& parser, shared_ptr<Node> node)
{
    return parseMeaning(parser, node, true);
}

bool MeaningMacro::expand(Parser& parser, shared_ptr<Node> node)
{
    return parseMeaning(parser, node, false);
}

bool ShowThe::invoke(Parser& parser, Node::ptr node)
{
    return parseThe(parser, node, true);
}

bool TheMacro::expand(Parser& parser, shared_ptr<Node> node)
{
    return parseThe(parser, node, false);
}

} // namespace base
} // namespace texpp

