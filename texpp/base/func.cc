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

#include <texpp/base/func.h>
#include <texpp/parser.h>

#include <boost/foreach.hpp>

namespace texpp {
namespace base {

bool Prefix::invokeWithPrefixes(Parser&, shared_ptr<Node>,
                                std::set<string>& prefixes)
{
    prefixes.insert(name());
    return true;
}

bool Assignment::invoke(Parser& parser, shared_ptr<Node> node)
{
    std::set<string> prefixes;
    return invokeWithPrefixes(parser, node, prefixes);
}

bool Assignment::checkPrefixes(Parser& parser,
        std::set<string> prefixes, bool macro)
{
    size_t ok = prefixes.count("\\global");
    bool global = ok;
    if(macro) {
        ok += prefixes.count("\\outer");
        ok += prefixes.count("\\long");
    }

    if(prefixes.size() != ok) {
        if(macro) {
            parser.logger()->log(Logger::ERROR,
                "You can't use such a prefix with `" + texRepr(&parser) + "'",
                parser, parser.lastToken());
        } else {
            string escape = parser.escapestr();
            parser.logger()->log(Logger::ERROR,
                string("You can't use `") + escape + "long' or `" +
                escape + "outer' with `" + texRepr(&parser) + "'",
                parser, parser.lastToken());
        }
    }

    int globaldefs = parser.symbol("globaldefs", int(0));
    if(globaldefs > 0) global = true;
    else if(globaldefs < 0) global = false;

    return global;
}

bool Let::invokeWithPrefixes(Parser& parser, shared_ptr<Node> node,
                                std::set<string>& prefixes)
{
    bool global = checkPrefixes(parser, prefixes);
    prefixes.clear();

    Node::ptr lvalue = parser.parseControlSequence(false);
    node->appendChild("lvalue", lvalue);

    Node::ptr equals(new Node("optional_equals"));
    node->appendChild("equals", equals);

    while(parser.helperIsImplicitCharacter(Token::CC_SPACE, false))
        parser.nextToken(&equals->tokens());

    if(parser.peekToken(false) && \
            parser.peekToken(false)->isCharacter('=', Token::CC_OTHER)) {
        equals->setValue(parser.nextToken(&equals->tokens()));
    }

    if(parser.helperIsImplicitCharacter(Token::CC_SPACE, false))
        parser.nextToken(&equals->tokens());

    Node::ptr rvalue = parser.parseToken(false);
    node->appendChild("rvalue", rvalue);

    Token::ptr ltoken = lvalue->value(Token::ptr());
    Token::ptr rtoken = rvalue->value(Token::ptr());

    if(rtoken->isControl()) {
        parser.setSymbol(
            ltoken, parser.symbol(rtoken, Command::ptr()), global);
    } else {
        parser.setSymbol(
            ltoken, Command::ptr(new TokenCommand(rtoken)), global);
    }

    return true;
}

bool Futurelet::invokeWithPrefixes(Parser& parser, Node::ptr node,
                                    std::set<string>& prefixes)
{
    bool global = checkPrefixes(parser, prefixes);
    prefixes.clear();

    Node::ptr lvalue = parser.parseControlSequence(false);
    node->appendChild("lvalue", lvalue);

    Node::ptr tokenNode = parser.parseToken(false);
    node->appendChild("token", tokenNode);

    Node::ptr rvalue = parser.parseToken(false);
    node->appendChild("rvalue", rvalue);

    Token::ptr ltoken = lvalue->value(Token::ptr());
    Token::ptr rtoken = rvalue->value(Token::ptr());
    Token::ptr token = tokenNode->value(Token::ptr());

    if(rtoken->isControl()) {
        parser.setSymbol(
            ltoken, parser.symbol(rtoken, Command::ptr()), global);
    } else {
        parser.setSymbol(
            ltoken, Command::ptr(new TokenCommand(rtoken)), global);
    }

    Token::list tokens;
    if(token)
        tokens.push_back(token->lcopy());
    if(rtoken)
        tokens.push_back(rtoken->lcopy());

    parser.pushBack(&tokens);

    return true;
}

bool Def::invokeWithPrefixes(Parser& parser, shared_ptr<Node> node,
                                std::set<string>& prefixes)
{
    bool global = checkPrefixes(parser, prefixes, true);
    bool longAttr = prefixes.count("\\long");
    bool outerAttr = prefixes.count("\\outer");
    prefixes.clear();

    global |= m_global;

    Node::ptr lvalue = parser.parseControlSequence(false);
    node->appendChild("lvalue", lvalue);
    Token::ptr ltoken = lvalue->value(Token::ptr());

    Node::ptr paramsNode(new Node("params"));
    node->appendChild("params", paramsNode);

    bool lbrace = false;
    int paramNum = 0;
    Token::list_ptr params(new Token::list);
    while(parser.peekToken(false)) {
        if(parser.peekToken(false)->isCharacterCat(Token::CC_BGROUP)) {
            break;
        } else if(parser.peekToken(false)->isCharacterCat(Token::CC_EGROUP)) {
            break;
        } else if(parser.peekToken(false)->isCharacterCat(Token::CC_PARAM)) {
            int curParamNum = -1;

            params->push_back(
                parser.nextToken(&paramsNode->tokens(), false));

            Token::ptr token = parser.peekToken(false);
            if(token && token->isCharacter()) {
                if(token->isCharacterCat(Token::CC_BGROUP)){
                    params->pop_back();
                    params->push_back(token->lcopy());
                    lbrace = true;
                    break;
                }
                char ch = token->value()[0];
                if(std::isdigit(ch)) curParamNum = ch - '0';
            }

            if(paramNum+1 > 9) {
                parser.logger()->log(Logger::ERROR,
                    "You already have nine parameters",
                    parser, parser.lastToken());
                params->pop_back();
            } else if(curParamNum != paramNum+1) {
                parser.logger()->log(Logger::ERROR,
                    "Parameters must be numbered consecutively",
                    parser, parser.lastToken());
                params->push_back(Token::create(
                        Token::TOK_CHARACTER, Token::CC_OTHER,
                        boost::lexical_cast<string>(++paramNum)));
            } else {
                params->push_back(
                    parser.nextToken(&paramsNode->tokens(), false));
                ++paramNum;
            }

        } else {
            params->push_back(parser.nextToken(&paramsNode->tokens(), false));
        }
    }

    paramsNode->setValue(params);

    Node::ptr left_brace(new Node("left_brace"));
    node->appendChild("left_brace", left_brace);

    if(parser.peekToken(false) && 
            parser.peekToken(false)->isCharacterCat(Token::CC_BGROUP)) {
        left_brace->setValue(parser.nextToken(&left_brace->tokens(), false));
    } else {
        parser.logger()->log(Logger::ERROR, "Missing { inserted",
            parser, parser.lastToken());
        left_brace->setValue(Token::create(
                    Token::TOK_CHARACTER, Token::CC_BGROUP, "{"));
    }

    // TODO: implement correct list expansion
    Node::ptr definition =
        parser.parseBalancedText(m_expand, paramNum, ltoken);

    /*
    if(m_expand) {
        Token::list_ptr tokens = definition->value(Token::list_ptr());
        Token::list_ptr tokens_copy(new Token::list());
        if(tokens) {
            BOOST_FOREACH(Token::ptr token, *tokens) {
                Command::ptr cmd = parser.symbol(token, Command::ptr());
                if(token->isControl() && !cmd) {
                    parser.logger()->log(Logger::ERROR,
                        "Undefined control sequence", parser, token);
                } else {
                    tokens_copy->push_back(token);
                }
            }
        }
        definition->setValue(tokens_copy);
    }
    */

    if(lbrace) {
        Token::list_ptr p = definition->value(Token::list_ptr());
        if(p) p->push_back(params->back());
    }

    node->appendChild("definition", definition);

    Node::ptr right_brace(new Node("right_brace"));
    node->appendChild("right_brace", right_brace);
    if(parser.peekToken(false) &&
            parser.peekToken(false)->isCharacterCat(Token::CC_EGROUP)) {
        right_brace->setValue(parser.nextToken(&right_brace->tokens(), false));
    } else {
        // TODO: error
        right_brace->setValue(Token::create(
                    Token::TOK_CHARACTER, Token::CC_EGROUP, "}"));
    }

    parser.setSymbol(ltoken,
        Command::ptr(new UserMacro(ltoken ? ltoken->value() : "\\undefined",
            paramsNode->value(Token::list_ptr()),
            definition->value(Token::list_ptr()),
            outerAttr, longAttr)), global);

    return true;
}

string UserMacro::texRepr(Parser* parser, bool newline, size_t limit) const
{
    string str;

    string escape = parser ? parser->escapestr() : string(1, '\\');
    if(m_longAttr) str = str + escape + "long";
    if(m_outerAttr) str = str + escape + "outer";
    if(!str.empty()) str += ' ';

    str += string("macro:") + (newline ? "\n" : "") +
            Token::texReprList(*m_params, parser, true) + "->";

    if(limit) limit = limit > str.size() ? limit - str.size() : 1;
    str += Token::texReprList(*m_definition, parser, true, limit);

    return str;
}

string UserMacro::texRepr(Parser* parser) const
{
    return texRepr(parser, true, 0);
}

bool UserMacro::expand(Parser& parser, shared_ptr<Node> node)
{
    // TODO: implement \long and \outer
    if(parser.symbol("tracingmacros", int(0)) > 0) {
        Token::ptr t = node->child("control_sequence")->value(Token::ptr());
        string str(1, '\n');
        str += //Token::texReprControl(name(), &parser, true) +
                Token::texReprControl(t ? t->value():name(), &parser, true) +
                Token::texReprList(*m_params, &parser, true) + "->" +
                Token::texReprList(*m_definition, &parser, true);
        parser.logger()->log(Logger::MTRACING,
            str, parser, Token::ptr());
    }

    Node::ptr child;

    Token::list_ptr params[9];
    size_t paramNum = 0;

    Token::list::iterator end = m_params->end();
    for(Token::list::iterator it = m_params->begin(); it < end; ++it) {
        if((*it)->isCharacterCat(Token::CC_PARAM)) {
            child = Node::ptr(new Node("arg"));
            node->appendChild("arg" +
                boost::lexical_cast<string>(paramNum+1), child);

            Token::list_ptr tokens(new Token::list());
            child->setValue(tokens);

            Token::ptr etoken;
            ++it;
            if(it+1 < end && !(*(it+1))->isCharacterCat(Token::CC_PARAM))
                etoken = *(it+1);

            if(!etoken) {
                while(parser.peekToken(false) &&
                        parser.helperIsImplicitCharacter(
                            Token::CC_SPACE, false))
                    parser.nextToken(&child->tokens());
            }


            int level = 0;
            Token::ptr token;
            while(token = parser.peekToken(false)) {
                if(level == 0 && etoken &&
                        token->type() == etoken->type() &&
                        token->catCode() == etoken->catCode() &&
                        token->value() == etoken->value()) {
                    break;
                } else if(token->isCharacterCat(Token::CC_BGROUP)) {
                    tokens->push_back(
                        parser.nextToken(&child->tokens(), false));
                    ++level;
                } else if(token->isCharacterCat(Token::CC_EGROUP)) {
                    tokens->push_back(
                        parser.nextToken(&child->tokens(), false));
                    --level;
                    if(level == 0 && !etoken) break;
                    if(level < 0) {
                        parser.logger()->log(Logger::ERROR,
                            "Argument of " + Command::texRepr(&parser) +
                            " has an extra }", parser, parser.lastToken());
                        level = 0;
                    }
                } else {
                    tokens->push_back(
                        parser.nextToken(&child->tokens(), false));
                    if(level == 0 && !etoken) break;
                }
            }

            if(tokens->size() >= 2 &&
                    tokens->front()->isCharacterCat(Token::CC_BGROUP) &&
                    tokens->back()->isCharacterCat(Token::CC_EGROUP)) {
                std::copy(tokens->begin()+1, tokens->end(),
                                tokens->begin());
                tokens->resize(tokens->size()-2);
            }

            params[paramNum++] = tokens;
            child->setValue(tokens);
            child.reset();

        } else {
            Token::ptr ntoken = parser.peekToken(false);
            if(!child) {
                child = Node::ptr(new Node("arg_skip"));
                node->appendChild("arg_skip", child);
            }
            parser.nextToken(&child->tokens(), false);

            if(!ntoken ||
                    ntoken->type() != (*it)->type() ||
                    ntoken->catCode() != (*it)->catCode() ||
                    ntoken->value() != (*it)->value()) {
                parser.logger()->log(Logger::ERROR,
                    "Use of " + Command::texRepr(&parser) +
                    " doesn't match its definition",
                    parser, parser.lastToken());
                return true;
            }
        }
    }

    if(parser.symbol("tracingmacros", int(0)) > 0) {
        for(size_t n = 0; n < paramNum; ++n) {
            string str("#");
            str += boost::lexical_cast<string>(n+1);
            str += "<-";
            if(params[n])
                str += Token::texReprList(*params[n], &parser);
            parser.logger()->log(Logger::MTRACING,
                str, parser, Token::ptr());
        }
    }

    Token::list_ptr result(new Token::list());
    result->reserve(m_definition->size());

    end = m_definition->end();
    for(Token::list::iterator it = m_definition->begin();
                                            it < end; ++it) {
        if((*it)->isCharacterCat(Token::CC_PARAM)) {
            if(++it >= end) break;
            if((*it)->isCharacterCat(Token::CC_PARAM)) {
                result->push_back((*it)->lineNo() ? (*it)->lcopy() : *it);
            } else if((*it)->isCharacter()) {
                char ch = (*it)->value()[0];
                if(!isdigit(ch) || ch == '0' || !params[ch-'0'-1]) continue;
                BOOST_FOREACH(Token::ptr token, *params[ch-'0'-1]) {
                    result->push_back(token->lineNo() ? token->lcopy() : token);
                }
            }
        } else {
            result->push_back((*it)->lineNo() ? (*it)->lcopy() : *it);
        }
    }

    node->setValue(result);
    return true;
}

} // namespace base
} // namespace texpp

