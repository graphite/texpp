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
#include <texpp/common.h>

namespace {
  template<typename T>
  struct shared_ptr_to_python_object
  {
    static PyObject* convert(const texpp::shared_ptr<T>& s)
    {
      if(!s) boost::python::incref(boost::python::object(T()).ptr());
      return boost::python::incref(boost::python::object(*s).ptr());
    }
  };
}

template<typename T>
void export_shared_ptr()
{
    boost::python::to_python_converter<
        texpp::shared_ptr<T>,
        shared_ptr_to_python_object<T> >();

    //python_object_to_boost_any::register_conversion();
}

