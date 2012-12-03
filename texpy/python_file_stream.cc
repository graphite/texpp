/*  *** License agreement ***
    
    cctbx Copyright (c) 2006, The Regents of the University of
    California, through Lawrence Berkeley National Laboratory (subject to
    receipt of any required approvals from the U.S. Dept. of Energy).  All
    rights reserved.
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    
    (1) Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    
    (2) Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    
    (3) Neither the name of the University of California, Lawrence Berkeley
    National Laboratory, U.S. Dept. of Energy nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
    OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    
    You are under no obligation whatsoever to provide any bug fixes,
    patches, or upgrades to the features, functionality or performance of
    the source code ("Enhancements") to anyone; however, if you choose to
    make your Enhancements available either publicly, or directly to
    Lawrence Berkeley National Laboratory, without imposing a separate
    written license agreement for such Enhancements, then you hereby grant
    the following license: a  non-exclusive, royalty-free perpetual license
    to install, use, modify, prepare derivative works, incorporate into
    other computer software, distribute, and sublicense such enhancements or
    derivative works thereof, in binary and source code form.
 */


#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/handle.hpp>

#include <boost/shared_ptr.hpp>

#include "python_file_stream.h"

namespace boost_adaptbx { namespace file_conversion {

  std::size_t python_file_buffer::buffer_size = 1024;

  // Boost.Python conversion dark magic (for python_file_buffer)
  struct python_file_to_stream_buffer
  {
    static void register_conversion() {
      using namespace boost::python;
      converter::registry::push_back(
        &convertible,
        &construct,
        type_id< boost::shared_ptr<python_file_buffer> >());
    }

    static void *convertible(PyObject *obj_ptr) {
      using namespace boost::python;
      if (!(   PyObject_HasAttrString(obj_ptr, "read")
            && PyObject_HasAttrString(obj_ptr, "readline")
            && PyObject_HasAttrString(obj_ptr, "readlines"))
          &&
          !(   PyObject_HasAttrString(obj_ptr, "write")
            && PyObject_HasAttrString(obj_ptr, "writelines"))) return 0;
      return obj_ptr;
    }

    static void construct(
      PyObject *obj_ptr,
      boost::python::converter::rvalue_from_python_stage1_data *data)
    {
      using namespace boost::python;
      typedef converter::rvalue_from_python_storage<
            boost::shared_ptr<python_file_buffer> > rvalue_t;
      void *storage = ((rvalue_t *) data)->storage.bytes;
      object python_file((handle<>(borrowed(obj_ptr))));
      new (storage) boost::shared_ptr<python_file_buffer>(
            new python_file_buffer(python_file));
      data->convertible = storage;
    }
  };

  // Boost.Python conversion dark magic (for stream)
  template<class _stream, class _realstream>
  struct python_file_to_stream
  {
    static void register_conversion() {
      using namespace boost::python;
      converter::registry::push_back(
        &convertible,
        &construct,
        type_id< boost::shared_ptr<_stream> >());
    }

    static void *convertible(PyObject *obj_ptr) {
      using namespace boost::python;
      if (obj_ptr != Py_None &&
          !(   PyObject_HasAttrString(obj_ptr, "read")
            && PyObject_HasAttrString(obj_ptr, "readline")
            && PyObject_HasAttrString(obj_ptr, "readlines"))
          &&
          !(   PyObject_HasAttrString(obj_ptr, "write")
            && PyObject_HasAttrString(obj_ptr, "writelines"))) return 0;
      return obj_ptr;
    }

    static void construct(
      PyObject *obj_ptr,
      boost::python::converter::rvalue_from_python_stage1_data *data)
    {
      using namespace boost::python;
      typedef converter::rvalue_from_python_storage<
            boost::shared_ptr<_stream> > rvalue_t;
      void *storage = ((rvalue_t *) data)->storage.bytes;
      if(obj_ptr == Py_None) {
          new (storage) boost::shared_ptr<_stream>();
      } else {
          object python_file((handle<>(borrowed(obj_ptr))));
          new (storage) boost::shared_ptr<_stream>(
                new _realstream(boost::shared_ptr<python_file_buffer>(
                    new python_file_buffer(python_file))));
      }
      data->convertible = storage;
    }
  };

  // register
  struct python_file_buffer_wrapper
  {
    typedef python_file_buffer wt;

    static void wrap() {
      using namespace boost::python;
      class_<wt, boost::shared_ptr<wt>, boost::noncopyable>("buffer", no_init)
        .def_readwrite("size", wt::buffer_size,
                       "The size of the buffer sitting "
                       "between a Python file object and a C++ stream.")
      ;
    }
  };

}} // boost_adaptbx::file_conversions

void export_python_stream()
{
  using namespace boost_adaptbx::file_conversion;
  python_file_to_stream_buffer::register_conversion();
  python_file_to_stream<std::istream, istream>::register_conversion();
  python_file_to_stream<std::ostream, ostream>::register_conversion();
  python_file_to_stream<std::iostream, iostream>::register_conversion();

  //python_file_buffer_wrapper::wrap();
}

