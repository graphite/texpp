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

#include <texpp/base/char.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <sstream>
#include <iomanip>

namespace texpp {
namespace base {

bool Char::invoke(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("char_number", number);
    int n = number->value(int(0));
    if(n < 0 || n > 255) {
        parser.logger()->log(Logger::ERROR,
            "Bad character code (" + boost::lexical_cast<string>(n) + ")",
            parser, parser.lastToken());
        n = 0;
    }
    return true;
}

bool MathChar::invoke(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("mathchar_number", number);
    int n = number->value(int(0));
    // TODO: check for math mode
    if(n < 0 || n > 32767) {
        parser.logger()->log(Logger::ERROR,
            "Bad mathchar (" + boost::lexical_cast<string>(n) + ")",
            parser, parser.lastToken());
        n = 0;
    }
    return true;
}

bool Delimiter::invoke(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("delimiter_number", number);
    int n = number->value(int(0));
    // TODO: check for math mode
    if(n < 0 || n > 134217727) {
        parser.logger()->log(Logger::ERROR,
            "Bad delimiter code (" + boost::lexical_cast<string>(n) + ")",
            parser, parser.lastToken());
        n = 0;
    }
    return true;
}

bool Char::createDef(Parser& parser, shared_ptr<Token> token,
                            int num, bool global)
{
    if(num < 0 || num > 255) {
        parser.logger()->log(Logger::ERROR,
            "Bad character code (" + boost::lexical_cast<string>(num) + ")",
            parser, parser.lastToken());
        num = 0;
    }

    std::ostringstream s; s << name() << '"' << std::hex << num;
    parser.setSymbol(token, Command::ptr(new CharDef(s.str(), num)), global);
    parser.setSymbolDefault(s.str().substr(1), int(0));
    return true;
}

bool MathChar::createDef(Parser& parser, shared_ptr<Token> token,
                            int num, bool global)
{
    if(num < 0 || num > 32767) {
        parser.logger()->log(Logger::ERROR,
            "Bad mathchar (" + boost::lexical_cast<string>(num) + ")",
            parser, parser.lastToken());
        num = 0;
    }

    std::ostringstream s;
    s << name() << '"' << std::hex << std::uppercase << num;
    parser.setSymbol(token, Command::ptr(new CharDef(s.str(), num)), global);
    parser.setSymbolDefault(s.str().substr(1), int(0));
    return true;
}

bool CharDef::invokeOperation(Parser&,
                shared_ptr<Node> node, Operation op, bool)
{
    if(op == GET) {
        node->setValue(m_initValue);
        return true;
    } else if(op == EXPAND) {
        node->setValue(boost::lexical_cast<string>(
            m_initValue.type() == typeid(int) ? 
            *unsafe_any_cast<int>(&m_initValue) : 0));
        return true;
    }
    return false;
}

} // namespace base
} // namespace texpp


