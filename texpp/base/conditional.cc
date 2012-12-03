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

#include <texpp/base/conditional.h>
#include <texpp/base/dimen.h>
#include <texpp/base/func.h>
#include <texpp/base/box.h>
#include <texpp/base/files.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <boost/lexical_cast.hpp>

namespace texpp {
namespace base {

bool Iftrue::evaluate(Parser&, shared_ptr<Node> node)
{
    node->setValue(true);
    return true;
}

bool Iffalse::evaluate(Parser&, shared_ptr<Node> node)
{
    node->setValue(false);
    return true;
}

bool Ifnum::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number1 = parser.parseNumber();
    node->appendChild("number1", number1);

    char relchar;
    Node::ptr relation(new Node("relation"));
    node->appendChild("relation", relation);

    Token::ptr token = parser.peekToken();
    if(token->isCharacter('<', Token::CC_OTHER) ||
            token->isCharacter('=', Token::CC_OTHER) ||
            token->isCharacter('>', Token::CC_OTHER)) {
        relchar = token->value()[0];
        parser.nextToken(&relation->tokens());
    } else {
        parser.logger()->log(Logger::ERROR,
            "Missing = inserted for " + texRepr(&parser),
            parser, parser.lastToken());
        relchar = '=';
    }
    relation->setValue(relchar);

    Node::ptr number2 = parser.parseNumber();
    node->appendChild("number2", number2);

    int n1 = number1->value(int(0));
    int n2 = number2->value(int(0));
    bool res = false;
    
    if(relchar == '<') res = n1 < n2;
    else if(relchar == '=') res = n1 == n2;
    else if(relchar == '>') res = n1 > n2;

    node->setValue(res);
    return true;
}

bool Ifdim::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr dimen1 = parser.parseDimen();
    node->appendChild("dimen1", dimen1);

    char relchar;
    Node::ptr relation(new Node("relation"));
    node->appendChild("relation", relation);

    Token::ptr token = parser.peekToken();
    if(token->isCharacter('<', Token::CC_OTHER) ||
            token->isCharacter('=', Token::CC_OTHER) ||
            token->isCharacter('>', Token::CC_OTHER)) {
        relchar = token->value()[0];
        parser.nextToken(&relation->tokens());
    } else {
        parser.logger()->log(Logger::ERROR,
            "Missing = inserted for " + texRepr(&parser),
            parser, parser.lastToken());
        relchar = '=';
    }
    relation->setValue(relchar);

    Node::ptr dimen2 = parser.parseDimen();
    node->appendChild("dimen2", dimen2);

    int n1 = dimen1->value(Dimen(0)).value;
    int n2 = dimen2->value(Dimen(0)).value;
    bool res = false;
    
    if(relchar == '<') res = n1 < n2;
    else if(relchar == '=') res = n1 == n2;
    else if(relchar == '>') res = n1 > n2;

    node->setValue(res);
    return true;
}

bool Ifodd::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);

    bool res = number->value(int(0)) % 2;
    node->setValue(res);
    return true;
}

bool Ifmode::evaluate(Parser& parser, shared_ptr<Node> node)
{
    for(int m=0; m<=Parser::DMATH; ++m) {
        if((m_modeMask & (1<<m)) && parser.mode() == m) {
            node->setValue(true);
            return true;
        }
    }
    node->setValue(false);
    return true;
}

bool Ifcat::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr token1Node = parser.parseToken();
    Token::ptr token1 = token1Node->value(Token::ptr());
    node->appendChild("token1", token1Node);

    Node::ptr token2Node = parser.parseToken();
    Token::ptr token2 = token2Node->value(Token::ptr());
    node->appendChild("token2", token2Node);

    int ch1 = 256, ch2 = 256;
    Token::CatCode cc1 = Token::CC_NONE, cc2 = Token::CC_NONE;

    
    TokenCommand::ptr tcmd1 = parser.symbolCommand<TokenCommand>(token1);
    if(tcmd1) token1 = tcmd1->token();

    TokenCommand::ptr tcmd2 = parser.symbolCommand<TokenCommand>(token2);
    if(tcmd2) token2 = tcmd2->token();

    if(token1->isCharacter()) {
        ch1 = token1->value()[0];
        cc1 = token1->catCode();
    }

    if(token2->isCharacter()) {
        ch2 = token2->value()[0];
        cc2 = token2->catCode();
    }

    bool res;
    if(m_testCat) res = cc1 == cc2;
    else res = ch1 == ch2;

    node->setValue(res);
    return true;
}

bool Ifx::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr token1Node = parser.parseToken(false);
    Token::ptr token1 = token1Node->value(Token::ptr());
    node->appendChild("token1", token1Node);

    Node::ptr token2Node = parser.parseToken(false);
    Token::ptr token2 = token2Node->value(Token::ptr());
    node->appendChild("token2", token2Node);

    TokenCommand::ptr tcmd1 = parser.symbolCommand<TokenCommand>(token1);
    if(tcmd1) token1 = tcmd1->token();

    TokenCommand::ptr tcmd2 = parser.symbolCommand<TokenCommand>(token2);
    if(tcmd2) token2 = tcmd2->token();

    bool res = false;
    if(token1->isCharacter() && token2->isCharacter()) {
        res = token1->catCode() == token2->catCode() &&
              token1->value() == token2->value();

    } else if(token1->isControl() && token2->isControl()) {
        Command::ptr cmd1 = parser.symbol(token1, Command::ptr());
        Command::ptr cmd2 = parser.symbol(token2, Command::ptr());

        if(cmd1 == cmd2) {
            res = true;

        } else if(dynamic_pointer_cast<Variable>(cmd1) &&
                  dynamic_pointer_cast<Variable>(cmd2)) {

            res = static_pointer_cast<Variable>(cmd1)->name() ==
                  static_pointer_cast<Variable>(cmd2)->name();

        } else if(dynamic_pointer_cast<UserMacro>(cmd1) &&
                  dynamic_pointer_cast<UserMacro>(cmd2)) {

            shared_ptr<UserMacro> um1 = static_pointer_cast<UserMacro>(cmd1);
            shared_ptr<UserMacro> um2 = static_pointer_cast<UserMacro>(cmd2);

            if(um1->outerAttr() == um2->outerAttr() &&
                    um1->longAttr() == um2->longAttr() &&
                    um1->params().size() == um2->params().size() &&
                    um1->definition().size() == um2->definition().size()) {
                res = true;
                for(size_t i=0; res && i<um1->params().size(); ++i) {
                    Token::ptr tok1 = um1->params()[i];
                    Token::ptr tok2 = um2->params()[i];
                    if(tok1->type() != tok2->type() ||
                            tok1->catCode() != tok2->catCode() ||
                            tok1->value() != tok2->value())
                        res = false;
                }
                for(size_t i=0; res && i<um1->definition().size(); ++i) {
                    Token::ptr tok1 = um1->definition()[i];
                    Token::ptr tok2 = um2->definition()[i];
                    if(tok1->type() != tok2->type() ||
                            tok1->catCode() != tok2->catCode() ||
                            tok1->value() != tok2->value())
                        res = false;
                }
            }
        }
    }

    node->setValue(res);
    return true;
}

bool Ifvoid::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);

    string s = "box" + boost::lexical_cast<string>(number->value(int(0)));
    node->setValue(bool(!parser.symbol(s, Box()).value));
    return true;
}

bool Ifhbox::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);

    string s = "box" + boost::lexical_cast<string>(number->value(int(0)));
    Box box = parser.symbol(s, Box());
    node->setValue(bool(box.value && box.mode == Parser::RHORIZONTAL));
    return true;
}

bool Ifvbox::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);

    string s = "box" + boost::lexical_cast<string>(number->value(int(0)));
    Box box = parser.symbol(s, Box());
    node->setValue(bool(box.value && box.mode == Parser::RVERTICAL));
    return true;
}

bool Ifcase::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);
    node->setValue(int(number->value(int(0))));
    return true;
}

bool Ifeof::evaluate(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);

    int stream = number->value(int(0));
    if(stream < 0 || stream > 15) {
        parser.logger()->log(Logger::ERROR,
                "Bad number (" + boost::lexical_cast<string>(stream) + ")",
                parser, parser.lastToken());
        stream = 0;
    }

    InFile infile = 
       parser.symbol("read" + boost::lexical_cast<string>(stream), InFile());

    bool res = !bool(infile.lexer);
    node->setValue(res);
    return true;
}

} // namespace base
} // namespace texpp

