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

#include <utility>

namespace {

  template<typename T1, typename T2>
  struct std_pair_to_python_tuple
  {
    static PyObject* convert(const std::pair<T1, T2>& s)
    {
      return boost::python::incref(
        boost::python::make_tuple(s.first, s.second).ptr());
    }
  };

  template<typename T1, typename T2>
  struct python_tuple_to_std_pair
  {
    static void register_conversion() {
      using namespace boost::python;
      converter::registry::push_back(
        &convertible,
        &construct,
        type_id< std::pair<T1, T2> >());
    }

    static void *convertible(PyObject *obj_ptr) {
      using namespace boost::python;
      if(!PySequence_Check(obj_ptr) ||
          PySequence_Size(obj_ptr) != 2) return 0;

      PyObject* p1 = PySequence_ITEM(obj_ptr, 0);
      PyObject* p2 = PySequence_ITEM(obj_ptr, 1);
      extract<T1> e1(p1);
      extract<T2> e2(p2);

      if(!e1.check() || !e2.check()) {
          Py_XDECREF(p1); Py_XDECREF(p2); return 0;
      }

      Py_XDECREF(p1); Py_XDECREF(p2); return obj_ptr;
    }

    static void construct(
      PyObject *obj_ptr,
      boost::python::converter::rvalue_from_python_stage1_data *data)
    {
      using namespace boost::python;
      typedef converter::rvalue_from_python_storage<
                            std::pair<T1, T2> > rvalue_t;
      void *storage = ((rvalue_t *) data)->storage.bytes;

      PyObject* p1 = PySequence_ITEM(obj_ptr, 0);
      PyObject* p2 = PySequence_ITEM(obj_ptr, 1);

      new (storage) std::pair<T1, T2>(
            extract<T1>(p1), extract<T2>(p2)
      );

      Py_XDECREF(p1); Py_XDECREF(p2);

      data->convertible = storage;
    }
  };
}

template<typename T1, typename T2>
void export_std_pair()
{
    using namespace boost::python;

    to_python_converter<
        std::pair<T1, T2>,
        std_pair_to_python_tuple<T1, T2> >();

    python_tuple_to_std_pair<T1, T2>::register_conversion();
}

