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
#include <texpp/parser.h>
#include <texpp/logger.h>

namespace texpp { namespace {

using boost::python::wrapper;
using boost::python::override;

template<class Log>
class LoggerWrap: public Log, public wrapper<Log>
{
public:
    bool log(Logger::Level level, const string& message,
                    Parser& parser, shared_ptr<Token> token) {
        if(override f = this->get_override("log"))
            return f(level, message, parser, token);
        return this->Log::log(level, message, parser, token);
    }

    bool default_log(Logger::Level level, const string& message,
                    Parser& parser, shared_ptr<Token> token) {
        return this->Log::log(level, message, parser, token);
    }
};

}}

void export_logger_base()
{
    using namespace boost::python;
    using namespace texpp;

    scope scopeLogger = class_<LoggerWrap<Logger>,
            shared_ptr<Logger>, boost::noncopyable>("Logger", no_init)

        /*.def("levelName", &Logger::levelName,
            return_value_policy<copy_const_reference>())*/
        .def("tokenLines", &Logger::tokenLines)
        .def("log", pure_virtual(&Logger::log))
        ;

    enum_<Logger::Level>("Level")
        .value("MESSAGE", Logger::MESSAGE)
        .value("SHOW", Logger::SHOW)
        .value("ERROR", Logger::ERROR)
        .value("CRITICAL", Logger::CRITICAL)
        .value("UNIMPLEMENTED", Logger::UNIMPLEMENTED)
        ;
}

template<class Log, class _bases>
void export_derived_logger(const char* name)
{
    using namespace boost::python;
    using namespace texpp;

    class_<LoggerWrap<Log>, shared_ptr<Log>, _bases >(name)
        .def("log", &Log::log,
                &LoggerWrap<Log>::default_log)
        ;
}

void export_logger()
{
    using namespace boost::python;
    using namespace texpp;

    export_logger_base();
    export_derived_logger<NullLogger, bases<Logger> >("NullLogger");
    export_derived_logger<ConsoleLogger, bases<Logger> >("ConsoleLogger");
}

