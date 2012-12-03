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

#ifndef __TEXPP_BASE_DIMEN_H
#define __TEXPP_BASE_DIMEN_H

#include <texpp/common.h>
#include <texpp/command.h>

#include <texpp/base/variable.h>
#include <boost/tuple/tuple.hpp>

namespace texpp {

using boost::tuple;

namespace base {

struct Dimen
{
    int value;
    Dimen() {}
    explicit Dimen(int v): value(v) {}
};

class InternalDimen: public Variable
{
public:
    InternalDimen(const string& name, const any& initValue = any(Dimen(0)))
        : Variable(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);

    static tuple<int,int,bool> multiplyIntFrac(int x, int n, int d);
    static string dimenToString(Dimen nn, int o=0, bool mu=false);
};

class DimenVariable: public InternalDimen
{
public:
    DimenVariable(const string& name, const any& initValue = any(Dimen(0)))
        : InternalDimen(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

class SpecialDimen: public InternalDimen
{
public:
    SpecialDimen(const string& name, const any& initValue = any(Dimen(0)))
        : InternalDimen(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

class BoxDimen: public InternalDimen
{
public:
    BoxDimen(const string& name,
        const any& initValue = any())
        : InternalDimen(name, initValue) {}

    string parseName(Parser& parser, shared_ptr<Node> node);
    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

} // namespace base
} // namespace texpp

#endif

