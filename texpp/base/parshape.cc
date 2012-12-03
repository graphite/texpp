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

#include <texpp/base/parshape.h>
#include <texpp/parser.h>
#include <texpp/logger.h>

#include <sstream>

namespace texpp {
namespace base {

bool Parshape::invokeOperation(Parser& parser,
                        shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());

        Node::ptr countNode = parser.parseNumber();
        node->appendChild("count", countNode);
        int count = countNode->value(int(0));

        ParshapeInfo info;
        for(int n=0; n<count; ++n) {
            Node::ptr dimen1 = parser.parseDimen();
            Node::ptr dimen2 = parser.parseDimen();
            node->appendChild("dimen_" +
                boost::lexical_cast<string>(n) + "_1", dimen1);
            node->appendChild("dimen_" +
                boost::lexical_cast<string>(n) + "_2", dimen2);
            info.parshape.push_back(std::make_pair(
                dimen1->value(int(0)), dimen2->value(int(0))));
        }

        node->setValue(info);
        parser.setSymbol(name, info, global);
        return true;

    } else if(op == GET) {
        string name = parseName(parser, node);
        ParshapeInfo info = parser.symbol(name, ParshapeInfo());
        node->setValue(info.parshape.size());
        return true;

    } else if(op == EXPAND) {
        string name = parseName(parser, node);
        ParshapeInfo info = parser.symbol(name, ParshapeInfo());
        node->setValue(boost::lexical_cast<string>(info.parshape.size()));
        return true;

    }

    return false;
}

} // namespace base
} // namespace texpp


