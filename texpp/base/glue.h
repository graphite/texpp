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

#ifndef __TEXPP_BASE_GLUE_H
#define __TEXPP_BASE_GLUE_H

#include <texpp/common.h>
#include <texpp/command.h>

#include <texpp/base/variable.h>
#include <texpp/base/dimen.h>
#include <boost/tuple/tuple.hpp>

namespace texpp {

namespace base {

struct Glue
{
    bool mu;
    Dimen width;
    Dimen stretch;
    int stretchOrder;
    Dimen shrink;
    int shrinkOrder;

    Glue() {}
    explicit Glue(bool m,
            int w, Dimen st = Dimen(0), int sto = 0,
            Dimen sh = Dimen(0), int sho = 0)
        : mu(m), width(w), stretch(st), stretchOrder(sto),
          shrink(sh), shrinkOrder(sho) {}
};

class InternalGlue: public Variable
{
public:
    InternalGlue(const string& name, const any& initValue = any(Glue(0,0)))
        : Variable(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);

    static string glueToString(const Glue& g);
};

class GlueVariable: public InternalGlue
{
public:
    GlueVariable(const string& name, const any& initValue = any(Glue(0,0)))
        : InternalGlue(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

class InternalMuGlue: public Variable
{
public:
    InternalMuGlue(const string& name, const any& initValue = any(Glue(1,0)))
        : Variable(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

class MuGlueVariable: public InternalMuGlue
{
public:
    MuGlueVariable(const string& name, const any& initValue = any(Glue(1,0)))
        : InternalMuGlue(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

} // namespace base
} // namespace texpp

#endif


