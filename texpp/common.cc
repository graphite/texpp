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

#include <texpp/common.h>
#include <texpp/token.h>
#include <texpp/parser.h>

#include <texpp/base/dimen.h>
#include <texpp/base/glue.h>

#include <sstream>
#include <iomanip>
#include <cassert>
#include <exception>

#include <boost/any.hpp>

namespace texpp {

pair<int,bool> safeMultiply(int v1, int v2, int max)
{
    if(v1 < 0) { v1=-v1; v2=-v2; }
    if(v1 != 0) {
        if(v2 > max/v1 || -v2 > max/v1)
            return std::make_pair(v1, true);
        v1 *= v2;
    }
    return std::make_pair(v1, false);
}

pair<int,bool> safeDivide(int v1, int v2)
{
    if(v2==0) return std::make_pair(v1, true);
    return std::make_pair(v1/v2, false);
}

string reprString(const string& s)
{
    std::ostringstream r;
    r << '\"';
    texpp::string::const_iterator end = s.end();
    for(texpp::string::const_iterator c = s.begin(); c != end; ++c) {
        if(*c == '\n') r << "\\n";
        else if(*c == '\r') r << "\\r";
        else if(*c == '\t') r << "\\t";
        else if(*c == '\0') r << "\\0";
        else if(*c == '\"') r << "\\\"";
        else if(*c == '\\') r << "\\\\";
        else if(*c < 0x20 || *c >= 0x7f)
            r << "\\x" << std::setfill('0') << std::setw(2)
                       << std::hex << (unsigned int)(*c);
        else
            r << *c;
    }
    r << '\"';
    return r.str();
}

string reprAny(const boost::any& value)
{
    using boost::unsafe_any_cast;

    std::ostringstream r;

    if(value.empty()) {
        r << "None";

    } else if(value.type() == typeid(int)) {
        r << *unsafe_any_cast<int>(&value);

    } else if(value.type() == typeid(short)) {
        r << *unsafe_any_cast<short>(&value);

    } else if(value.type() == typeid(long)) {
        r << *unsafe_any_cast<long>(&value);

    } else if(value.type() == typeid(string)) {
        r << reprString(*unsafe_any_cast<string>(&value));

    } else if(value.type() == typeid(pair<int,int>)) {
        pair<int,int> v = *unsafe_any_cast<pair<int,int> >(&value);
        r << "(" << v.first << ", " << v.second << ")";

    } else if(value.type() == typeid(base::Dimen)) {
        r << base::InternalDimen::dimenToString(
                *unsafe_any_cast<base::Dimen>(&value));

    } else if(value.type() == typeid(base::Glue)) {
        r << base::InternalGlue::glueToString(
                *unsafe_any_cast<base::Glue>(&value));

    } else if(value.type() == typeid(Token::ptr)) {
        Token::ptr tok = (*unsafe_any_cast<Token::ptr>(&value));
        r << (tok ? tok->texRepr() : "null");

    } else if(value.type() == typeid(Command::ptr)) {
        Command::ptr cmd = (*unsafe_any_cast<Command::ptr>(&value));
        r << (cmd ? cmd->texRepr() : "null");

    } else if(value.type() == typeid(Token::list_ptr)) {
        r << "TokenList("
          << (*unsafe_any_cast<Token::list_ptr>(&value))->size()
          << " tokens)";
    } else {
        r << "any(" << value.type().name() << "())";
    }
    return r.str();
}


} // namespace texpp

namespace boost {

void throw_exception(std::exception const &) {
    assert(false);
}

} // namespace boost

