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

#ifndef __TEXPP_LEXER_H
#define __TEXPP_LEXER_H

#include <texpp/common.h>
#include <texpp/token.h>

#include <istream>

namespace texpp {

class Lexer;
class Context;

class Lexer
{
public:
    Lexer(const string& fileName, std::istream* file,
                bool interactive = false, bool saveLines = false);
    Lexer(const string& fileName, shared_ptr<std::istream> file,
                bool interactive = false, bool saveLines = false);
    ~Lexer();

    Token::ptr nextToken();

    bool interactive() const { return m_interactive; }

    string jobName() const;
    const string& fileName() const { return *m_fileName; }
    shared_ptr<string> fileNamePtr() const { return m_fileName; }

    size_t linePos() const { return m_linePos; }
    size_t lineNo() const { return m_lineNo; }
    const string& line() const { return m_lineOrig; }
    const string& line(size_t n) const;

    int endlinechar() const { return m_endlinechar; }
    void setEndlinechar(int endlinechar) { m_endlinechar = endlinechar; }

    int catcode(int ch) const { return m_catcode[ch]; }
    void setCatcode(int ch, int code) { m_catcode[ch] = code; }

protected:
    void init();

    Token::ptr newToken(Token::Type type,
                    const string& value = string());

    bool nextLine();
    bool nextChar();

protected:
    enum State {
        ST_EOF = 0,
        ST_EOL = 1,
        ST_NEW_LINE = 2,
        ST_SKIP_SPACES = 3,
        ST_MIDDLE = 4
    };

    shared_ptr<std::istream> m_fileShared;

    std::istream*   m_file;
    shared_ptr<string> m_fileName;

    string  m_lineOrig;
    string  m_lineTex;

    size_t  m_linePos;
    size_t  m_lineNo;
    size_t  m_charPos;
    size_t  m_charEnd;

    State   m_state;
    int     m_char;
    Token::CatCode
            m_catCode;

    int     m_endlinechar;
    char    m_catcode[256];

    bool    m_interactive;
    bool    m_saveLines;

    vector<string> m_lines;
};

} // namespace

#endif

