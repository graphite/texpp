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

#include <texpp/logger.h>
#include <texpp/token.h>
#include <texpp/lexer.h>
#include <texpp/parser.h>

#include <iostream>
#include <sstream>
#include <algorithm>

#include <boost/foreach.hpp>

namespace {
/*texpp::string loggerLevelNames[] = {
    "message", "show", "error", "critical"
};*/
const unsigned int MAX_LINE_CHARS = 79;
const unsigned int MAX_TLINE_CHARS = 50;
} // namespace

namespace texpp {

/*
const string& Logger::levelName(Level level) const
{
    if(level <= MESSAGE) return loggerLevelNames[0];
    else if(level <= SHOW) return loggerLevelNames[1];
    else if(level <= ERROR) return loggerLevelNames[2];
    else return loggerLevelNames[3];
}
*/

string Logger::tokenLines(Parser& parser, Token::ptr token) const
{
    if(!token || !token->lineNo()) return string();

    std::ostringstream r;
    if(parser.lexer()->fileName().empty()) r << "<*> ";
    else r << "l." << token->lineNo() << " ";

    if(token->fileName() == parser.lexer()->fileName()) {
        const string& line = parser.lexer()->line(token->lineNo());
        if(!line.empty()) {
            string line1 = line.substr(0, token->charEnd());
            if(!line1.empty() && line1[line1.size()-1] == '\n')
                line1 = line1.substr(0, line1.size()-1);
            int d = line1.size() + r.str().size() - MAX_TLINE_CHARS;
            if(d > 0)
                line1 = "..." + line1.substr(d+3, line1.size()-d-3);
            r << line1 << '\n';
            
            string line2 = line.substr(token->charEnd(),
                    line.find_last_not_of(" \r\n") + 1 - token->charEnd());
            if(line2.size() + r.str().size() - 1 > MAX_LINE_CHARS)
                line2 = line2.substr(0,
                    MAX_LINE_CHARS - r.str().size() - 2) + "...";
            r << string(r.str().size()-1, ' ') << line2 << '\n';
        }
    }

    return r.str();
}

ConsoleLogger::~ConsoleLogger()
{
    if(m_linePos) std::cout << std::endl;
}

bool ConsoleLogger::log(Level level, const string& message,
                            Parser& parser, Token::ptr token)
{
    /*
    if(level <= TRACING && parser.symbol("tracingonline", int(0)) <= 0)
        return true;
    */

    std::ostringstream r;
    
    if(level <= MTRACING) {
        //if(m_linePos) r << '\n';
        r << message;// << std::endl;
    } else if(level <= TRACING) {
        if(m_linePos) r << '\n';
        r << '{' << message << '}';// << std::endl;
    } else if(level <= PLAIN) {
        r << message;
    } else if(level <= MESSAGE) {
        if(m_linePos) r << ' '; 
        r << message;
    } else if(level <= WRITE) {
        if(m_linePos) r << '\n'; 
        r << message << '\n';
    } else {
        if(m_linePos) r << '\n';
        if(level <= SHOW) r << "> " << message << ".\n";
        else r << "! " << message << ".\n";
        if(token && token->lineNo())
            r << tokenLines(parser, token) << "\n";
    }

    std::ostringstream r1;
    int newlinechar = parser.symbol("newlinechar", int(0));
    BOOST_FOREACH(unsigned char ch, r.str()) {
        if(ch == '\n' || ch == newlinechar) {
            r1 << '\n';
        } else if(ch <= 0x1f) {
            r1 << "^^" << char(ch+64);
        } else if(ch >= 0x7f) {
            r1 << "^^" << std::hex << int(ch);
        } else {
            r1 << ch;
        }
    }

    BOOST_FOREACH(unsigned char ch, r1.str()) {
        if(m_linePos >= MAX_LINE_CHARS) {
            std::cout << '\n';
            m_linePos = 0;
        }
        std::cout << ch;
        if(ch == '\n')
            m_linePos = 0;
        else
            ++m_linePos;
    }

    std::cout << std::flush;

    if(m_linePos && (parser.lexer()->interactive() ||
                        level <= TRACING)) {
        std::cout << std::endl;
        m_linePos = 0;
    }

    return true;
}

} // namespace texpp

