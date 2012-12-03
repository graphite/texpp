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

#ifndef __TEXPP_COMMON_H
#define __TEXPP_COMMON_H

#include <string>
#include <vector>

#include <iostream> // tmp

//#include <tr1/memory>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

#include <tr1/unordered_map>

#ifndef WINDOWS
#define PATH_SEP '/'
#else
#define PATH_SEP '\\'
#endif

#define TEXPP_INT_INV    (int(-0x80000000))
#define TEXPP_INT_MIN    (int(-0x7fffffff))
#define TEXPP_INT_MAX    (int( 0x7fffffff))
#define TEXPP_SCALED_MAX (int( 0x3fffffff))

namespace texpp {
    using std::string;
    using std::vector;
    using std::pair;
    using std::tr1::unordered_map;
    //using std::tr1::shared_ptr;
    //using std::tr1::weak_ptr;

    using boost::shared_ptr;
    using boost::weak_ptr;
    using boost::dynamic_pointer_cast;
    using boost::static_pointer_cast;

    using boost::any;
    using boost::any_cast;
    using boost::unsafe_any_cast;

    pair<int,bool> safeMultiply(int v1, int v2, int max);
    pair<int,bool> safeDivide(int v1, int v2);

    string reprString(const string& s);
    string reprAny(const any& value);
} // namespace texpp

namespace std { namespace tr1 {
    template<typename T> struct hash< texpp::shared_ptr<T> > {
        size_t operator()(const texpp::shared_ptr<T>& v) const {
            return hash< void* >()( v.get() );
        }
    };
}} // namespace tr1 // namespace std

#endif

