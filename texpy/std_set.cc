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
#include <set>
#include <string>

namespace {
  template<typename T>
  struct std_set_to_python_object
  {
    static PyObject* convert(const std::set<T>& s)
    {
      PyObject* pySet = PySet_New(NULL);
      typename std::set<T>::const_iterator end = s.end();
      for(typename std::set<T>::const_iterator it = s.begin();
                                            it != end; ++it) {
          PySet_Add(pySet, boost::python::object(*it).ptr());
      }
      return pySet;
    }
  };
}

void export_std_set()
{
    boost::python::to_python_converter<
        std::set<std::string>,
        std_set_to_python_object<std::string> >();

    //python_object_to_boost_any::register_conversion();
}

