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

#include <texpp/command.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <boost/foreach.hpp>

namespace texpp {

string Command::texRepr(Parser* parser) const
{
    return Token::texReprControl(m_name, parser);
}

string Command::repr() const
{
    return "Command(" + reprString(name())
            + ", " + reprString(texRepr()) + ")";
}

string TokenCommand::texRepr(Parser* parser) const
{
    return m_token->meaning(parser);
}

bool TokenCommand::invoke(Parser&, shared_ptr<Node> node)
{
    node->setValue(m_token);
    return true;
}

Token::list_ptr Macro::stringToTokens(const string& str)
{
    Token::list_ptr ret(new Token::list());

    BOOST_FOREACH(char ch, str) {
        Token::CatCode catcode = Token::CC_OTHER;
        if(ch == ' ') catcode = Token::CC_SPACE;

        ret->push_back(Token::create(
            Token::TOK_CHARACTER, catcode, string(1, ch)));
    }

    return ret;
}

} // namespace texpp

