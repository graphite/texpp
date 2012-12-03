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

#ifndef __TEXPP_BASE_MESSAGE_H
#define __TEXPP_BASE_MESSAGE_H

#include <texpp/common.h>
#include <texpp/command.h>
#include <texpp/base/func.h>
#include <texpp/lexer.h>

namespace texpp {
namespace base {

struct InFile
{
    shared_ptr<Lexer> lexer;
    explicit InFile(shared_ptr<Lexer> l = shared_ptr<Lexer>())
        : lexer(l) {}
};

struct OutFile
{
    shared_ptr<std::ostream> ostream;
    explicit OutFile(shared_ptr<std::ostream> s = shared_ptr<std::ostream>())
        : ostream(s) {}
};

class Openin: public Command
{
public:
    explicit Openin(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class Closein: public Command
{
public:
    explicit Closein(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class Read: public Assignment
{
public:
    explicit Read(const string& name): Assignment(name) {}
    bool invokeWithPrefixes(Parser& parser, shared_ptr<Node>,
                                    std::set<string>& prefixes);
};

class Immediate: public Prefix
{
public:
    explicit Immediate(const string& name = string()): Prefix(name) {}
    bool invokeWithPrefixes(Parser& parser, shared_ptr<Node>,
                                    std::set<string>& prefixes);
};

class Openout: public Command
{
public:
    explicit Openout(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
    bool invokeWithPrefixes(Parser&, shared_ptr<Node>,
                                std::set<string>& prefixes);
};

class Closeout: public Command
{
public:
    explicit Closeout(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
    bool invokeWithPrefixes(Parser&, shared_ptr<Node>,
                                std::set<string>& prefixes);
};

class Write: public Command
{
public:
    explicit Write(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
    bool invokeWithPrefixes(Parser&, shared_ptr<Node>,
                                std::set<string>& prefixes);
};

class Message: public Command
{
public:
    explicit Message(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class Input: public Command
{
public:
    explicit Input(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class Endinput: public Command
{
public:
    explicit Endinput(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

} // namespace base
} // namespace texpp


#endif

