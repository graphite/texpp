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

#include <texpp/base/glue.h>
#include <texpp/base/dimen.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <sstream>

namespace texpp {
namespace base {

namespace {
bool invokeGlueOperation(Variable& var, Parser& parser,
        shared_ptr<Node> node, Variable::Operation op, bool global, bool mu)
{
    if(op == Variable::ASSIGN) {
        string name = var.parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());
        Node::ptr rvalue = parser.parseGlue(mu);
        node->appendChild("rvalue", rvalue);

        node->setValue(rvalue->valueAny());
        parser.setSymbol(name, rvalue->valueAny(), global);
        return true;
    } else if(op == Variable::EXPAND) {
        string name = var.parseName(parser, node);
        Glue val = parser.symbol(name, Glue(mu,0));
        node->setValue(InternalGlue::glueToString(val));
        return true;
    } else {
        return var.Variable::invokeOperation(parser, node, op, global);
    }
}

bool invokeGlueVarOperation(Variable& var, Parser& parser,
        shared_ptr<Node> node, Variable::Operation op, bool global, bool mu)
{
    static vector<string> kw_by(1, "by");
    if(op == Variable::ADVANCE) {
        string name = var.parseName(parser, node);
        
        node->appendChild("by", parser.parseOptionalKeyword(kw_by));

        Node::ptr rvalue = parser.parseGlue(mu);
        node->appendChild("rvalue", rvalue);

        Glue v = parser.symbol(name, Glue(mu,0));
        Glue rv = rvalue->value(Glue(mu,0));

        v.width.value += rv.width.value;

        if(v.stretch.value == 0) v.stretchOrder = 0;
        if(v.stretchOrder == rv.stretchOrder) {
            v.stretch.value += rv.stretch.value;
        } else if(v.stretchOrder < rv.stretchOrder && rv.stretch.value != 0) {
            v.stretch.value = rv.stretch.value;
            v.stretchOrder = rv.stretchOrder;
        }

        if(v.shrink.value == 0) v.shrinkOrder = 0;
        if(v.shrinkOrder == rv.shrinkOrder) {
            v.shrink.value += rv.shrink.value;
        } else if(v.shrinkOrder < rv.shrinkOrder && rv.shrink.value != 0) {
            v.shrink.value = rv.shrink.value;
            v.shrinkOrder = rv.shrinkOrder;
        }

        node->setValue(v);
        parser.setSymbol(name, v, global);
        return true;

    } else if(op == Variable::MULTIPLY || op == Variable::DIVIDE) {
        string name = var.parseName(parser, node);

        node->appendChild("by", parser.parseOptionalKeyword(kw_by));

        Node::ptr rvalue = parser.parseNumber();
        node->appendChild("rvalue", rvalue);

        Glue v = parser.symbol(name, Glue(mu,0));
        int rv = rvalue->value(int(0));
        bool overflow = false;

        if(op == Variable::MULTIPLY) {
            pair<int,bool> p = safeMultiply(
                v.width.value, rv, TEXPP_SCALED_MAX);
            v.width.value = p.first; overflow |= p.second;

            p = safeMultiply(v.stretch.value, rv, TEXPP_SCALED_MAX);
            v.stretch.value = p.first; overflow |= p.second;

            p = safeMultiply(v.shrink.value, rv, TEXPP_SCALED_MAX);
            v.shrink.value = p.first; overflow |= p.second;

        } else if(op == Variable::DIVIDE) {
            pair<int,bool> p = safeDivide(v.width.value, rv);
            v.width.value = p.first; overflow |= p.second;

            p = safeDivide(v.stretch.value, rv);
            v.stretch.value = p.first; overflow |= p.second;

            p = safeDivide(v.shrink.value, rv);
            v.shrink.value = p.first; overflow |= p.second;
        }

        if(overflow) {
            parser.logger()->log(Logger::ERROR,
                "Arithmetic overflow",
                parser, parser.lastToken());
        } else {
            node->setValue(v);
            parser.setSymbol(name, v, global);
        }
        return true;
    }

    return invokeGlueOperation(var, parser, node, op, global, mu);
}
}

bool InternalGlue::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    return invokeGlueOperation(*this, parser, node, op, global, false);
}

bool GlueVariable::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    return invokeGlueVarOperation(*this, parser, node, op, global, false);
}

bool InternalMuGlue::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    return invokeGlueOperation(*this, parser, node, op, global, true);
}

bool MuGlueVariable::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    return invokeGlueVarOperation(*this, parser, node, op, global, true);
}

string InternalGlue::glueToString(const Glue& g)
{
    string s = InternalDimen::dimenToString(g.width, 0, g.mu);
    if(g.stretch.value) {
        s = s + " plus " +
            InternalDimen::dimenToString(g.stretch, g.stretchOrder, g.mu);
    }
    if(g.shrink.value) {
        s = s + " minus " +
            InternalDimen::dimenToString(g.shrink, g.shrinkOrder, g.mu);
    }
    return s;
}

} // namespace base
} // namespace texpp


