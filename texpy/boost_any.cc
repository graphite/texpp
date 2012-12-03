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
#include <boost/any.hpp>

#include <texpp/command.h>

#include <string>

namespace {

  struct boost_any_to_python_object
  {
    static PyObject* convert(const boost::any& s)
    {
      using namespace boost::python;
      using namespace texpp;

      if(s.empty())
          return incref(object().ptr());
      else if(s.type() == typeid(int))
          return incref(object(*boost::unsafe_any_cast<int>(&s)).ptr());
      else if(s.type() == typeid(short))
          return incref(object(*boost::unsafe_any_cast<short>(&s)).ptr());
      else if(s.type() == typeid(long))
          return incref(object(*boost::unsafe_any_cast<long>(&s)).ptr());
      else if(s.type() == typeid(std::string))
          return incref(object(*boost::unsafe_any_cast<std::string>(&s)).ptr());
      else if(s.type() == typeid(object))
          return incref(boost::unsafe_any_cast<object>(&s)->ptr());
      else if(s.type() == typeid(Command::ptr))
          return incref(object(boost::unsafe_any_cast<Command::ptr>(&s)).ptr());
      else
          return incref(object(reprAny(s)).ptr());
    }

  };

  struct python_object_to_boost_any
  {
    static void register_conversion() {
      using namespace boost::python;
      converter::registry::push_back(
        &convertible,
        &construct,
        type_id< boost::any >());
    }

    static void *convertible(PyObject *obj_ptr) {
      return obj_ptr;
    }

    static void construct(
      PyObject *obj_ptr,
      boost::python::converter::rvalue_from_python_stage1_data *data)
    {
      using namespace boost::python;
      using namespace texpp;

      typedef converter::rvalue_from_python_storage<
                            boost::any > rvalue_t;
      void *storage = ((rvalue_t *) data)->storage.bytes;

      if(obj_ptr == Py_None) {
          // Empty
          new (storage) boost::any();
      } else if(PyInt_Check(obj_ptr)) {
          // Int
          new (storage) boost::any(int(PyInt_AS_LONG(obj_ptr)));
      } else if(PyString_Check(obj_ptr)) {
          // String
          new (storage) boost::any(std::string(
            PyString_AS_STRING(obj_ptr), PyString_GET_SIZE(obj_ptr) ));
      } else {
          // try extract Command
          object any_object((handle<>(borrowed(obj_ptr))));

          extract<Command::ptr> cmd(any_object);
          if(cmd.check()) {
              new (storage) boost::any(Command::ptr(cmd));
          } else {
              // fallback
              new (storage) boost::any(any_object);
          }
      }

      data->convertible = storage;
    }
  };
}

void export_boost_any()
{
    using namespace boost::python;

    to_python_converter<
        boost::any,
        boost_any_to_python_object>();

    python_object_to_boost_any::register_conversion();
}

