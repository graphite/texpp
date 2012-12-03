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

#ifndef __TEXPP_BASE_FUNC_H
#define __TEXPP_BASE_FUNC_H

#include <texpp/common.h>
#include <texpp/command.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <iostream>

namespace texpp {
namespace base {

// TODO: move this class to command.h
class Prefix: public Command
{
public:
    explicit Prefix(const string& name): Command(name) {}
    bool invoke(Parser&, shared_ptr<Node>) { return false; }
    bool invokeWithPrefixes(Parser&, shared_ptr<Node>,
                                std::set<string>& prefixes);
};

class AssignmentPrefix: public Prefix
{
public:
    explicit AssignmentPrefix(const string& name = string()): Prefix(name) {}
};

class Assignment: public Command
{
public:
    explicit Assignment(const string& name = string()): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);

protected:
    bool checkPrefixes(Parser& parser,
            std::set<string> prefixes, bool macro = false);
};

class Let: public Assignment
{
public:
    explicit Let(const string& name): Assignment(name) {}
    bool invokeWithPrefixes(Parser&, shared_ptr<Node>,
                                std::set<string>& prefixes);
};

class Futurelet: public Assignment
{
public:
    explicit Futurelet(const string& name): Assignment(name) {}
    bool invokeWithPrefixes(Parser&, shared_ptr<Node>,
                                std::set<string>& prefixes);
};

template<class Cmd>
class RegisterDef: public Assignment
{
public:
    RegisterDef(const string& name, shared_ptr<Cmd> group)
        : Assignment(name), m_group(group) {}

    shared_ptr<Cmd> group() { return m_group; }

    bool invokeWithPrefixes(Parser& parser, shared_ptr<Node> node,
                                std::set<string>& prefixes);

protected:
    shared_ptr<Cmd> m_group;
};

template<class Cmd>
bool RegisterDef<Cmd>::invokeWithPrefixes(Parser& parser,
                shared_ptr<Node> node, std::set<string>& prefixes)
{
    bool global = checkPrefixes(parser, prefixes);
    prefixes.clear();

    Node::ptr lvalue = parser.parseControlSequence(false);
    node->appendChild("lvalue", lvalue);
    Token::ptr ltoken = lvalue->value(Token::ptr());
    parser.lockToken(ltoken);

    node->appendChild("equals", parser.parseOptionalEquals());

    Node::ptr rvalue = parser.parseNumber();
    node->appendChild("rvalue", rvalue);

    int num = rvalue->value(int(0));

    parser.lockToken(Token::ptr());
    return m_group->createDef(parser, ltoken, num, global);
}

class Def: public Assignment
{
public:
    explicit Def(const string& name,
                    bool global = false, bool expand = false)
        : Assignment(name), m_global(global), m_expand(expand) {}
    bool invokeWithPrefixes(Parser& parser, shared_ptr<Node> node,
                                std::set<string>& prefixes);

    bool global() const { return m_global; }
    bool expand() const { return m_expand; }

protected:
    bool m_global;
    bool m_expand;
};

class UserMacro: public Macro
{
public:
    explicit UserMacro(const string& name,
        Token::list_ptr params, Token::list_ptr definition,
        bool outerAttr = false, bool longAttr = false)
        : Macro(name), m_params(params), m_definition(definition),
          m_outerAttr(outerAttr), m_longAttr(longAttr) {}

    Token::list params() { return *m_params; }
    Token::list definition() { return *m_definition; }
    bool outerAttr() const { return m_outerAttr; }
    bool longAttr() const { return m_longAttr; }

    string texRepr(Parser* parser,
            bool newline, size_t limit = 0) const;
    string texRepr(Parser* parser = NULL) const;
    bool expand(Parser& parser, shared_ptr<Node> node);

protected:
    Token::list_ptr m_params;
    Token::list_ptr m_definition;
    bool m_outerAttr;
    bool m_longAttr;
};

} // namespace base
} // namespace texpp


#endif

