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

#ifndef __TEXPP_BASE_CONDITIONAL_H
#define __TEXPP_BASE_CONDITIONAL_H

#include <texpp/common.h>
#include <texpp/command.h>

namespace texpp {
namespace base {

class Iftrue: public ConditionalBegin
{
public:
    Iftrue(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser&, shared_ptr<Node> node);
};

class Iffalse: public ConditionalBegin
{
public:
    Iffalse(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser&, shared_ptr<Node> node);
};

class Ifnum: public ConditionalBegin
{
public:
    Ifnum(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);
};

class Ifdim: public ConditionalBegin
{
public:
    Ifdim(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);
};

class Ifodd: public ConditionalBegin
{
public:
    Ifodd(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);
};

class Ifmode: public ConditionalBegin
{
public:
    Ifmode(const string& name = string(), int modeMask = 0)
        : ConditionalBegin(name), m_modeMask(modeMask) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);

protected:
    int m_modeMask;
};

class Ifcat: public ConditionalBegin
{
public:
    Ifcat(const string& name = string(), bool testCat = true)
        : ConditionalBegin(name), m_testCat(testCat) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);

protected:
    bool m_testCat;
};

class Ifx: public ConditionalBegin
{
public:
    Ifx(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);
};

class Ifvoid: public ConditionalBegin
{
public:
    Ifvoid(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);
};

class Ifhbox: public ConditionalBegin
{
public:
    Ifhbox(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);
};

class Ifvbox: public ConditionalBegin
{
public:
    Ifvbox(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);
};

class Ifcase: public ConditionalBegin
{
public:
    Ifcase(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);
};

class Ifeof: public ConditionalBegin
{
public:
    Ifeof(const string& name = string()): ConditionalBegin(name) {}
    bool evaluate(Parser& parser, shared_ptr<Node> node);
};

} // namespace base
} // namespace texpp

#endif

