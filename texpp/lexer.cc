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

#include <texpp/lexer.h>

#include <iostream>

namespace texpp {

Lexer::Lexer(const string& fileName, std::istream* file,
                bool interactive, bool saveLines)
    : m_fileShared(), m_file(file),
      m_fileName(new string(fileName)),
      m_linePos(0), m_lineNo(0), m_charPos(0), m_charEnd(0),
      m_state(ST_NEW_LINE), m_char(-1), m_catCode(Token::CC_NONE),
      m_interactive(interactive), m_saveLines(saveLines)
{
    if(!m_file) { m_file = &std::cin; }
    init();
}

Lexer::Lexer(const string& fileName, shared_ptr<std::istream> file,
                    bool interactive, bool saveLines)
    : m_fileShared(file), m_file(file.get()),
      m_fileName(new string(fileName)),
      m_linePos(0), m_lineNo(0), m_charPos(0), m_charEnd(0),
      m_state(ST_NEW_LINE), m_char(-1), m_catCode(Token::CC_NONE),
      m_interactive(interactive), m_saveLines(saveLines)
{
    if(!m_file) { m_file = &std::cin; }
    init();
}

Lexer::~Lexer()
{
}

void Lexer::init()
{
    m_endlinechar = '\r';
    for(int i=0; i<256; ++i)
        m_catcode[i] = Token::CC_OTHER;

    for(int i='a'; i<='z'; ++i)
        m_catcode[i] = Token::CC_LETTER;

    for(int i='A'; i<='Z'; ++i)
        m_catcode[i] = Token::CC_LETTER;

    m_catcode[0x7f] = Token::CC_INVALID;
    m_catcode['\\'] = Token::CC_ESCAPE;
    m_catcode['\r'] = Token::CC_EOL;
    m_catcode[' '] = Token::CC_SPACE;
    m_catcode['%'] = Token::CC_COMMENT;
}

string Lexer::jobName() const
{
    string jobname(*m_fileName);

    size_t n = jobname.rfind(PATH_SEP);
    if(n != jobname.npos)
        jobname = jobname.substr(n+1);
    n = jobname.rfind(".tex");
    if(n != jobname.npos)
        jobname = jobname.substr(0, n);
    return jobname;
}

const string& Lexer::line(size_t n) const
{
    static string EMPTY_LINE;
    if(n-1 < m_lines.size()) return m_lines[n-1];
    else return EMPTY_LINE;
}

bool Lexer::nextLine()
{
    m_charPos = 0;
    m_charEnd = 0;

    m_char = -1;
    m_catCode = Token::CC_NONE;

    m_linePos += m_lineOrig.size();
    m_lineOrig.clear();

    if(m_interactive && m_file == &std::cin) {
        std::cout << "*";
    }

    // Scan line until '\n' or '\r' or '\r\n'
    while(true) {
        char c = m_file->get();
        if(!m_file->good()) // TODO: handle errors
            break;

        m_lineOrig.push_back(c);
        if(c == '\n') {
            break;
        } else if(c == '\r') {
            if(m_file->peek() == '\n')
                m_lineOrig.push_back(char(m_file->get()));
            break;
        }
    }

    // Check EOF
    if(m_lineOrig.empty()) {
        m_lineTex.clear();
        return false;
    }

    if(m_saveLines)
        m_lines.push_back(m_lineOrig);

    // Discard spaces at the end
    size_t end = m_lineOrig.find_last_not_of(" \r\n");
    if(end == string::npos) end = -1;

    m_lineTex.reserve(end+2);
    m_lineTex.assign(m_lineOrig, 0, end+1);

    // Append endlinechar
    if(m_endlinechar >= 0 && m_endlinechar <= 255)
        m_lineTex += char(m_endlinechar);

    // Finalize
    ++m_lineNo;
    return true;
}

bool Lexer::nextChar()
{
    m_charPos = m_charEnd;

    if(m_charPos >= m_lineTex.size()) {
        m_char = -1;
        m_catCode = Token::CC_EOL;
        return false;
    }

    m_char = m_lineTex[m_charPos];
    m_catCode = Token::CatCode(m_catcode[(unsigned char) m_char]);

    if(m_catCode == Token::CC_SUPER && m_charPos+2 < m_lineTex.size() &&
                                    m_lineTex[m_charPos+1] == m_char) {
        if(m_charPos+3 < m_lineTex.size() &&
                std::isxdigit(m_lineTex[m_charPos+2]) &&
                std::isxdigit(m_lineTex[m_charPos+3]) &&
                !std::isupper(m_lineTex[m_charPos+2]) &&
                !std::isupper(m_lineTex[m_charPos+3])) {
            char c1 = m_lineTex[m_charPos+2];
            char c2 = m_lineTex[m_charPos+3];
            m_char = (isdigit(c1) ? c1-'0' : c1-'a'+10) * 16 +
                     (isdigit(c2) ? c2-'0' : c2-'a'+10);
            m_charEnd = m_charPos+4;
        } else {
            m_char = (m_lineTex[m_charPos+2] + 64) & 0x7f;
            m_charEnd = m_charPos+3;
        }
        m_catCode = Token::CatCode(m_catcode[(unsigned char) m_char]);
    } else {
        m_charEnd = m_charPos + 1;
    }

    if(m_charEnd >= m_lineTex.size())
        m_charEnd = std::max(m_charEnd, m_lineOrig.size());

    return true;
}

inline Token::ptr Lexer::newToken(Token::Type type,
                                const string& value)
{
    return Token::create(
        type, m_catCode, 
        value.empty() && m_char >= 0 ? string(1, m_char) : value,
        m_lineOrig.substr(std::min(m_charPos, m_lineOrig.size()),
                    m_charEnd - m_charPos),
                m_linePos,
                m_lineNo,
                std::min(m_charPos, m_lineOrig.size()),
                std::min(m_charEnd, m_lineOrig.size()),
                m_charEnd >= m_lineTex.size(),
                m_fileName);
}

Token::ptr Lexer::nextToken()
{
    if(m_state == ST_EOF)
        return Token::ptr();

    while(true) {
        if(!nextChar())
            m_state = ST_EOL;

        /////////// Handle ST_EOL
        if(m_state == ST_EOL) {

            if(m_charPos < m_lineOrig.size()) {
                m_charEnd = m_lineOrig.size();
                return newToken(Token::TOK_SKIPPED);
            }

            if(!nextLine()) {
                m_state = ST_EOF;
                return Token::ptr();
            }

            m_state = ST_NEW_LINE;
            continue;
        }

        /////////// Handle ST_NEW_LINE and ST_SKIP_SPACES
        else if(m_state == ST_NEW_LINE || m_state == ST_SKIP_SPACES) {

            //// CC_EOL
            if(m_catCode == Token::CC_EOL) {
                if(m_state == ST_SKIP_SPACES) {
                    m_state = ST_EOL;
                    m_charEnd = m_charPos;
                } else {
                    m_state = ST_EOL;
                    return newToken(Token::TOK_CONTROL, "\\par");
                }
            }
            //// CC_IGNORED
            else if(m_catCode == Token::CC_IGNORED) {
                return newToken(Token::TOK_SKIPPED);
            }
            //// CC_INVALID
            else if(m_catCode == Token::CC_INVALID) {
                return newToken(Token::TOK_SKIPPED);
            }
            //// !(CC_EOL || CC_SPACE || CC_IGNORED || CC_INVALID)
            else if(m_catCode != Token::CC_SPACE) {
                m_state = ST_MIDDLE;
                m_charEnd = m_charPos;
                continue;
            }
            //// CC_SPACE
            else {
                Token::ptr token = newToken(Token::TOK_SKIPPED);
                while(nextChar() && m_catCode == Token::CC_SPACE) {}
                m_charEnd = m_charPos;
                token->setCharEnd(std::min(m_charEnd, m_lineOrig.size()));
                token->setSource(m_lineOrig.substr(
                    std::min(token->charPos(), m_lineOrig.size()),
                    token->charEnd() - token->charPos()));
                return token;
            }
        }

        /////////// Handle ST_MIDDLE
        else if(m_state == ST_MIDDLE) {

            //// CC_ESCAPE
            if(m_catCode == Token::CC_ESCAPE) {
                Token::ptr token = newToken(Token::TOK_CONTROL, "\\");
                string value("\\");

                if(nextChar()) {
                    value += char(m_char);

                    if(m_catCode == Token::CC_LETTER) {
                        while(nextChar() && m_catCode == Token::CC_LETTER)
                            value += char(m_char);

                        m_state = ST_SKIP_SPACES;
                        m_charEnd = m_charPos;

                    } else if(m_catCode == Token::CC_SPACE) {
                        m_state = ST_SKIP_SPACES;
                    }

                    token->setValue(value);
                    token->setCharEnd(std::min(m_charEnd, m_lineOrig.size()));
                    token->setSource(m_lineOrig.substr(
                        std::min(token->charPos(), m_lineOrig.size()),
                        token->charEnd() - token->charPos()));
                }

                return token;
            }
            //// CC_ACTIVE
            else if(m_catCode == Token::CC_ACTIVE) {
                return newToken(Token::TOK_CONTROL,
                                    string("`") + char(m_char));
            }
            //// CC_SPACE
            else if(m_catCode == Token::CC_SPACE) {
                m_state = ST_SKIP_SPACES;
                return newToken(Token::TOK_CHARACTER, string(1, ' '));
            }
            //// CC_EOL
            else if(m_catCode == Token::CC_EOL) {
                m_state = ST_EOL;
                m_catCode = Token::CC_SPACE;
                return newToken(Token::TOK_CHARACTER, string(1, ' '));
            }
            //// CC_COMMENT
            else if(m_catCode == Token::CC_COMMENT) {
                m_state = ST_EOL;
                m_charEnd = m_charPos;
                continue;
            }
            //// CC_IGNORED
            else if(m_catCode == Token::CC_IGNORED) {
                return newToken(Token::TOK_SKIPPED);
            }
            //// CC_INVALID
            else if(m_catCode == Token::CC_INVALID) {
                return newToken(Token::TOK_SKIPPED);
            }
            //// CC_BGROUP CC_EGROUP CC_MATHSHIFT CC_ALIGNTAB CC_PARAM
            //// CC_SUPER CC_SUB CC_LETTER CC_OTHER
            else {
                return newToken(Token::TOK_CHARACTER);
            }
        }
    }
}

} // namespace texpp

