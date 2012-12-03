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

#include <texpp/base/misc.h>
#include <texpp/base/parshape.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

namespace texpp {
namespace base {

bool UnimplementedCommand::invoke(Parser& parser, shared_ptr<Node> node)
{
    parser.logger()->log(Logger::UNIMPLEMENTED,
        "Command " +
        node->child("control_sequence")->value(Token::ptr())->texRepr(&parser) +
        " is not yet implemented in TeXpp",
        parser, parser.lastToken());
    return true;
}

bool End::invoke(Parser& parser, shared_ptr<Node>)
{
    parser.end();
    return true;
}

bool Par::invoke(Parser& parser, shared_ptr<Node>)
{
    parser.resetParagraphIndent();
    if(parser.mode() == Parser::RHORIZONTAL)
        parser.setMode(Parser::RVERTICAL);
    else if(parser.mode() == Parser::HORIZONTAL)
        parser.setMode(Parser::VERTICAL);
    return true;
}

bool Changecase::invoke(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr text = parser.parseGeneralText(false, false);
    node->appendChild("text", text);

    Token::list_ptr tokens =
        text->child("balanced_text")->value(Token::list_ptr());

    if(tokens) {
        Token::list newTokens;
        BOOST_FOREACH(Token::ptr token, *tokens) {
            Token::ptr newToken = token->lcopy();

            if(token->isCharacter()) {
                int newCode = parser.symbol(m_table +
                        boost::lexical_cast<string>(int(token->value()[0])),
                        int(0));
                if(newCode > 0 && newCode <= 255)
                    newToken->setValue(string(1, char(newCode)));
            } else if(token->isControl() && token->value().substr(0,1)=="`"){
                int newCode = parser.symbol(m_table +
                        boost::lexical_cast<string>(int(token->value()[1])),
                        int(0));
                if(newCode > 0 && newCode <= 255)
                    newToken->setValue("`" + string(1, char(newCode)));
            }

            newTokens.push_back(newToken);
        }

        parser.pushBack(&newTokens);
    }

    return true;
}

bool SetInteraction::invoke(Parser& parser, shared_ptr<Node>)
{
    parser.setInteraction(m_interaction);
    return true;
}

bool Afterassignment::invoke(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr tokenNode = parser.parseToken(false);
    node->appendChild("token", tokenNode);

    Token::ptr token = tokenNode->value(Token::ptr());
    if(token)
        parser.setAfterassignmentToken(token->lcopy());

    return true;
}

bool Aftergroup::invoke(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr tokenNode = parser.parseToken(false);
    node->appendChild("token", tokenNode);

    Token::ptr token = tokenNode->value(Token::ptr());
    if(token)
        parser.addAftergroupToken(token->lcopy());

    return true;
}

} // namespace base
} // namespace texpp

