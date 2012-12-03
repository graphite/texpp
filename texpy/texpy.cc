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

#include <boost/python.hpp>
#include <texpp/token.h>
#include <texpp/kpsewhich.h>

#include <iostream>
#include <istream>
#include <memory>

void export_python_stream();
void export_boost_any();
void export_std_set();
void export_token();
void export_lexer();
void export_command();
void export_parser();
void export_logger();

BOOST_PYTHON_MODULE(texpy)
{
    using namespace boost::python;
    using namespace texpp;

    export_python_stream();
    export_boost_any();
    export_std_set();
    export_token();
    export_lexer();
    export_command();
    export_parser();
    export_logger();

    def("kpsewhich", texpp::kpsewhich);

}

