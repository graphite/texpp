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

#ifndef __TEXPP_BASE_VARIABLES_H
#define __TEXPP_BASE_VARIABLES_H

#include <texpp/common.h>
#include <texpp/command.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <texpp/base/func.h>

namespace texpp {
namespace base {

class Variable: public Assignment
{
public:
    enum Operation { GET, ASSIGN, ADVANCE, MULTIPLY, DIVIDE, EXPAND };

    Variable(const string& name, const any& initValue = any())
        : Assignment(name), m_initValue(initValue) {}

    const any& initValue() const { return m_initValue; }

    virtual string parseName(Parser& parser, shared_ptr<Node> node);
    virtual bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);

    bool invokeWithPrefixes(Parser& parser, shared_ptr<Node> node,
                                std::set<string>& prefixes);

    template<class Var>
    static Node::ptr tryParseVariableValue(Parser& parser);

protected:
    any m_initValue;
};

class ArithmeticCommand: public Assignment
{
public:
    explicit ArithmeticCommand(const string& name, Variable::Operation op)
        : Assignment(name), m_op(op) {}

    Variable::Operation operation() const { return m_op; }

    bool invokeWithPrefixes(Parser& parser, shared_ptr<Node> node,
                                std::set<string>& prefixes);
protected:
    Variable::Operation m_op;
};

template<class Var>
class Register: public Var
{
public:
    Register(const string& name, const any& initValue)
        : Var(name, initValue) {}

    string parseName(Parser& parser, shared_ptr<Node> node);
    bool createDef(Parser& parser, shared_ptr<Token> token,
                            int num, bool global);
};

template<class Var>
class ReadOnlyVariable: public Var
{
public:
    ReadOnlyVariable(const string& name, const any& initValue = any(0))
        : Var(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Variable::Operation op, bool global);
};

template<class Var>
Node::ptr Variable::tryParseVariableValue(Parser& parser)
{
    shared_ptr<Var> var = parser.symbolCommand<Var>(parser.peekToken());
    if(!var) return Node::ptr();

    Node::ptr node(new Node("variable"));
    node->appendChild("control_token", parser.parseToken());
    if(var->invokeOperation(parser, node, base::Variable::GET, false))
        return node;

    parser.pushBack(&node->tokens());
    return Node::ptr();
}

template<class Var>
string Register<Var>::parseName(Parser& parser, shared_ptr<Node> node)
{
    shared_ptr<Node> number = parser.parseNumber();
    node->appendChild("variable_number", number);
    int n = number->value(int(0));

    if(n < 0 || n > 255) {
        parser.logger()->log(Logger::ERROR,
            "Bad register code (" + boost::lexical_cast<string>(n) + ")",
            parser, parser.lastToken());
        n = 0;
    }

    string s = this->name().substr(1) + boost::lexical_cast<string>(n);
    parser.setSymbolDefault(s, this->m_initValue);
    return s;
}

template<class Var>
bool Register<Var>::createDef(Parser& parser, Token::ptr token,
                                int num, bool global)
{
    if(num < 0 || num > 255) {
        parser.logger()->log(Logger::ERROR,
            "Bad register code (" + boost::lexical_cast<string>(num) + ")",
            parser, parser.lastToken());
        num = 0;
    }

    string iname = this->name() + boost::lexical_cast<string>(num);
    parser.setSymbol(token,
        Command::ptr(new Var(iname, this->initValue())), global);
    parser.setSymbolDefault(iname.substr(1), this->initValue());
    return true;
}

template<class Var>
bool ReadOnlyVariable<Var>::invokeOperation(Parser& parser,
            shared_ptr<Node> node, Variable::Operation op, bool global)
{
    if(op == Variable::ASSIGN) {
        parser.logger()->log(Logger::ERROR, "You can't use `" +
            this->texRepr(&parser) + "' in " + parser.modeName() + " mode",
            parser, parser.lastToken());
        return true;
    } else {
        return Var::invokeOperation(parser, node, op, global);
    }
}

} // namespace base
} // namespace texpp

#endif

