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

#ifndef __TEXPP_BASE_BOX_H
#define __TEXPP_BASE_BOX_H

#include <texpp/common.h>
#include <texpp/command.h>
#include <texpp/token.h>
#include <texpp/parser.h>

#include <texpp/base/variable.h>
#include <texpp/base/dimen.h>

namespace texpp {
namespace base {

struct Box
{
    Parser::Mode mode;
    bool top;

    Dimen width;
    Dimen height;
    Dimen skip;

    Token::list_ptr value;

    Box()
        : mode(Parser::RVERTICAL), top(false), width(0), height(0), skip(0) {}
    explicit Box(Parser::Mode m, bool t = false)
        : mode(m), top(t), width(0), height(0), skip(0) {}
};

class BoxVariable: public Variable
{
public:
    BoxVariable(const string& name,
        const any& initValue = any(Box()))
        : Variable(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

class Lastbox: public BoxVariable
{
public:
    Lastbox(const string& name,
        const any& initValue = any(Box()))
        : BoxVariable(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

class Vsplit: public Register<BoxVariable>
{
public:
    Vsplit(const string& name,
        const any& initValue = any(Box()))
        : Register<BoxVariable>(name, initValue) {}

    string parseName(Parser& parser, shared_ptr<Node> node);
};

class Setbox: public Variable
{
public:
    Setbox(const string& name,
        const any& initValue = any(Box()))
        : Variable(name, initValue) {}

    string parseName(Parser& parser, shared_ptr<Node> node);
    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

class BoxSpec: public BoxVariable
{
public:
    BoxSpec(const string& name, Parser::Mode mode, bool top)
        : BoxVariable(name, Box()), m_mode(mode), m_top(top) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
protected:
    Parser::Mode m_mode;
    bool m_top;
};

class Rule: public Command
{
public:
    explicit Rule(const string& name, Parser::Mode mode)
        : Command(name), m_mode(mode) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
    bool presetMode(Parser& parser);

protected:
    Parser::Mode m_mode;
};

} // namespace base
} // namespace texpp

#endif


