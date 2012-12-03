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

#ifndef __TEXPP_BASE_SHOW_H
#define __TEXPP_BASE_SHOW_H

#include <texpp/common.h>
#include <texpp/command.h>

namespace texpp {
namespace base {

class Show: public Command
{
public:
    explicit Show(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class ShowThe: public Command
{
public:
    explicit ShowThe(const string& name): Command(name) {}
    bool invoke(Parser& parser, shared_ptr<Node> node);
};

class MeaningMacro: public Macro
{
public:
    explicit MeaningMacro(const string& name): Macro(name) {}
    bool expand(Parser& parser, shared_ptr<Node> node);
};

class TheMacro: public Macro
{
public:
    explicit TheMacro(const string& name): Macro(name) {}
    bool expand(Parser& parser, shared_ptr<Node> node);
};

} // namespace base
} // namespace texpp


#endif

