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

#ifndef __TEXPP_BASE_CHAR_H
#define __TEXPP_BASE_CHAR_H

#include <texpp/common.h>
#include <texpp/command.h>
#include <texpp/base/integer.h>

namespace texpp {
namespace base {

class Char: public Command
{
public:
    explicit Char(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
    bool createDef(Parser& parser, shared_ptr<Token> token,
                        int num, bool global);
};

class MathChar: public Command
{
public:
    explicit MathChar(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
    bool createDef(Parser& parser, shared_ptr<Token> token,
                        int num, bool global);
};

class Delimiter: public Command
{
public:
    explicit Delimiter(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class CharDef: public InternalInteger
{
public:
    explicit CharDef(const string& name, const any& initValue = any(0))
        : InternalInteger(name, initValue) {}
    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

} // namespace base
} // namespace texpp


#endif


