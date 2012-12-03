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

#ifndef __TEXPP_BASE_MISC_H
#define __TEXPP_BASE_MISC_H

#include <texpp/common.h>
#include <texpp/command.h>
#include <texpp/parser.h>

namespace texpp {
namespace base {

class UnimplementedCommand: public Command
{
public:
    explicit UnimplementedCommand(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class IgnoredCommand: public Command
{
public:
    explicit IgnoredCommand(const string& name): Command(name) {}
};

class Relax: public Command
{
public:
    explicit Relax(const string& name): Command(name) {}
    bool invokeWithPrefixes(Parser&, shared_ptr<Node>,
                        std::set<string>&) { return true; }
};

class End: public Command
{
public:
    explicit End(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class Par: public Command
{
public:
    explicit Par(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class Changecase: public Command
{
public:
    explicit Changecase(const string& name, const string& table)
        : Command(name), m_table(table) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);

protected:
    string m_table;
};

class SetInteraction: public Command
{
public:
    explicit SetInteraction(const string& name, Parser::Interaction intr)
        : Command(name), m_interaction(intr) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);

protected:
    Parser::Interaction m_interaction;
};

class Afterassignment: public Command
{
public:
    explicit Afterassignment(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class Aftergroup: public Command
{
public:
    explicit Aftergroup(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

} // namespace base
} // namespace texpp


#endif

