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

#define BOOST_TEST_MODULE lexer_test_suite
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <texpp/lexer.h>
#include <iostream>
#include <sstream>

using namespace texpp;

shared_ptr<Lexer> create_lexer(const string& input)
{
    shared_ptr<std::istream> ifile(new std::istringstream(input));
    return shared_ptr<Lexer>(new Lexer("", ifile, false));
}

std::vector<Token::ptr> run_lexer(shared_ptr<Lexer> lexer)
{
    Token::ptr token;
    std::vector<Token::ptr> output;
    while(token = lexer->nextToken())
        output.push_back(token);

    return output;
}

void check_output(Token* tokens, size_t count, vector<Token::ptr> output,
                        bool print=false)
{
    using namespace boost::lambda;
    vector<string> tokens_repr(count);
    std::transform(tokens, tokens+count,
            tokens_repr.begin(), bind(&Token::repr, boost::lambda::_1));

    vector<string> output_repr(output.size());
    std::transform(output.begin(), output.end(),
            output_repr.begin(), bind(&Token::repr, *boost::lambda::_1));

    if(print)
        std::for_each(output_repr.begin(), output_repr.end(),
                        std::cout << boost::lambda::_1 << '\n');

    BOOST_CHECK_EQUAL_COLLECTIONS(
        tokens_repr.begin(), tokens_repr.end(),
        output_repr.begin(), output_repr.end());
}

BOOST_AUTO_TEST_CASE( lexer_empty )
{
    {
    shared_ptr<Lexer> lexer = create_lexer("");
    Token tokens[] = {
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer),1);
    }
}

BOOST_AUTO_TEST_CASE( lexer_physical_eol )
{
    {
    shared_ptr<Lexer> lexer = create_lexer("a\rb\nc\r\nd\n\re");
    Token tokens[] = {
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "a", "a", 0, 1, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "\r", 0, 1, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "b", "b", 2, 2, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "\n", 2, 2, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "c", "c", 4, 3, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "\r\n", 4, 3, 1, 3),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "d", "d", 7, 4, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "\n", 7, 4, 1, 2),
        Token(Token::TOK_CONTROL, Token::CC_EOL, "\\par", "\r", 9, 5, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "e", "e", 10, 6, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "", 10, 6, 1, 1),
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }

    {
    shared_ptr<Lexer> lexer = create_lexer("a\rb\nc\r\nd\n\re");
    lexer->setEndlinechar('A');
    Token tokens[] = {
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "a", "a", 0, 1, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "A", "\r", 0, 1, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "b", "b", 2, 2, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "A", "\n", 2, 2, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "c", "c", 4, 3, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "A", "\r\n", 4, 3, 1, 3),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "d", "d", 7, 4, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "A", "\n", 7, 4, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "A", "\r", 9, 5, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "e", "e", 10, 6, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "A", "", 10, 6, 1, 1),
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }

    {
    shared_ptr<Lexer> lexer = create_lexer("a\rb\nc\r\nd\n\re");
    lexer->setEndlinechar(-1);
    Token tokens[] = {
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "a", "a\r", 0, 1, 0, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "b", "b\n", 2, 2, 0, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "c", "c\r\n", 4, 3, 0, 3),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "d", "d\n", 7, 4, 0, 2),
        Token(Token::TOK_SKIPPED, Token::CC_EOL, "", "\r", 9, 5, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "e", "e", 10, 6, 0, 1),
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }
}

BOOST_AUTO_TEST_CASE( lexer_eol )
{
    {
    shared_ptr<Lexer> lexer = create_lexer("a\rb\nc\r\nd\n\re");
    lexer->setCatcode('\r', Token::CC_OTHER);
    Token tokens[] = {
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "a", "a", 0, 1, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "\r", "\r", 0, 1, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "b", "b", 2, 2, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "\r", "\n", 2, 2, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "c", "c", 4, 3, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "\r", "\r\n", 4, 3, 1, 3),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "d", "d", 7, 4, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "\r", "\n", 7, 4, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "\r", "\r", 9, 5, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "e", "e", 10, 6, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "\r", "", 10, 6, 1, 1),
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }

    {
    shared_ptr<Lexer> lexer = create_lexer("a\rb\nc\r\nd\n\re");
    lexer->setCatcode('c', Token::CC_EOL);
    Token tokens[] = {
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "a", "a", 0, 1, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "\r", 0, 1, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "b", "b", 2, 2, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "\n", 2, 2, 1, 2),
        Token(Token::TOK_CONTROL, Token::CC_EOL, "\\par", "c", 4, 3, 0, 1),
        Token(Token::TOK_SKIPPED, Token::CC_EOL, "\r", "\r\n", 4, 3, 1, 3),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "d", "d", 7, 4, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "\n", 7, 4, 1, 2),
        Token(Token::TOK_CONTROL, Token::CC_EOL, "\\par", "\r", 9, 5, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "e", "e", 10, 6, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "", 10, 6, 1, 1),
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }
}

BOOST_AUTO_TEST_CASE( lexer_special_chars )
{
    {
    shared_ptr<Lexer> lexer = create_lexer(
                "^ ^^ ^^^ ^^^^ ^^\n"     // standaline ^ and ^^\n
                "^^7^^7a^^7A^^7z\n"      // handling of both ^^-forms
                "^^5e^^5e7a\n"           // ^^ replaced only once
                "^^20^^20^^0d^^20a\n"    // spaces and newline
                "^^20^^`^^M x\n"         // another representation of spaces
                "^^\\\\^^7a\n"           // escape character
                "^^"                     // ^^ at the end of file
    );
    lexer->setCatcode('^', Token::CC_SUPER);
    Token tokens[] = {
        Token(Token::TOK_CHARACTER, Token::CC_SUPER, "^", "^", 0, 1, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", " ", 0, 1, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "`", "^^ ", 0, 1, 2, 5),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "\x1e", "^^^", 0, 1, 5, 8),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", " ", 0, 1, 8, 9),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "\x1e", "^^^", 0, 1, 9, 12),
        Token(Token::TOK_CHARACTER, Token::CC_SUPER, "^", "^", 0, 1, 12, 13),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", " ", 0, 1, 13, 14),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "M", "^^\n", 0, 1, 14, 17),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "w", "^^7", 17, 2, 0, 3),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "z", "^^7a", 17, 2, 3, 7),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "w", "^^7", 17, 2, 7, 10),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "A", "A", 17, 2, 10, 11),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "w", "^^7", 17, 2, 11, 14),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "z", "z", 17, 2, 14, 15),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "\n", 17, 2, 15, 16),
        Token(Token::TOK_CHARACTER, Token::CC_SUPER, "^", "^^5e", 33, 3, 0, 4),
        Token(Token::TOK_CHARACTER, Token::CC_SUPER, "^", "^^5e", 33, 3, 4, 8),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "7", "7", 33, 3, 8, 9),

        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "a", "a", 33, 3, 9, 10),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "\n", 33, 3, 10, 11),
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", "^^20^^20", 44, 4, 0, 8),
        Token(Token::TOK_CONTROL, Token::CC_EOL, "\\par", "^^0d", 44, 4, 8, 12),
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", "^^20a\n", 44, 4, 12, 18),
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", "^^20^^`", 62, 5, 0, 7),
        Token(Token::TOK_CONTROL, Token::CC_EOL, "\\par", "^^M", 62, 5, 7, 10),
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", " x\n", 62, 5, 10, 13),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, "\x1c", "^^\\", 75, 6, 0, 3),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\z", "\\^^7a", 75, 6, 3, 8),
        Token(Token::TOK_SKIPPED, Token::CC_EOL, "\r", "\n", 75, 6, 8, 9),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "M", "^^", 84, 7, 0, 2),
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }

    {
    shared_ptr<Lexer> lexer = create_lexer(
        "^^\n"  // should not be replaced
    );
    lexer->setCatcode('^', Token::CC_SUPER);
    lexer->setEndlinechar(-1);
    Token tokens[] = {
        Token(Token::TOK_CHARACTER, Token::CC_SUPER, "^", "^", 0, 1, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SUPER, "^", "^\n", 0, 1, 1, 3)
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }
}

BOOST_AUTO_TEST_CASE( lexer_control )
{
    {
    shared_ptr<Lexer> lexer = create_lexer(
        "\\abc  \\abc \\abc:\\abc\\a \\; \\\\ \\  \\~~ \\");
    lexer->setCatcode('~', Token::CC_ACTIVE);
    Token tokens[] = {
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\abc", "\\abc", 0, 1, 0, 4),
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", "  ", 0, 1, 4, 6),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\abc", "\\abc", 0, 1, 6, 10),
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", " ", 0, 1, 10, 11),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\abc", "\\abc", 0, 1, 11, 15),
        Token(Token::TOK_CHARACTER, Token::CC_OTHER, ":", ":", 0, 1, 15, 16),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\abc", "\\abc", 0, 1, 16, 20),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\a", "\\a", 0, 1, 20, 22),
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", " ", 0, 1, 22, 23),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\;", "\\;", 0, 1, 23, 25),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", " ", 0, 1, 25, 26),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\\\", "\\\\", 0, 1, 26, 28),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", " ", 0, 1, 28, 29),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\ ", "\\ ", 0, 1, 29, 31),
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", " ", 0, 1, 31, 32),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\~", "\\~", 0, 1, 32, 34),
        Token(Token::TOK_CONTROL, Token::CC_ACTIVE, "`~", "~", 0, 1, 34, 35),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", " ", 0, 1, 35, 36),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\\r", "\\", 0, 1, 36, 37),
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }
}

BOOST_AUTO_TEST_CASE( lexer_comment )
{
    {
    shared_ptr<Lexer> lexer = create_lexer(" abc\\%de%fg\\hk\nl");
    lexer->setCatcode(0, Token::CC_IGNORED);
    Token tokens[] = {
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", " ", 0, 1, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "a", "a", 0, 1, 1, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "b", "b", 0, 1, 2, 3),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "c", "c", 0, 1, 3, 4),
        Token(Token::TOK_CONTROL, Token::CC_ESCAPE, "\\%", "\\%", 0, 1, 4, 6),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "d", "d", 0, 1, 6, 7),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "e", "e", 0, 1, 7, 8),
        Token(Token::TOK_SKIPPED, Token::CC_COMMENT, "%", "%fg\\hk\n", 0, 1, 8, 15),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "l", "l", 15, 2, 0, 1),
        Token(Token::TOK_CHARACTER, Token::CC_SPACE, " ", "", 15, 2, 1, 1),
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }
}


BOOST_AUTO_TEST_CASE( lexer_other )
{
    {
    shared_ptr<Lexer> lexer = create_lexer(string("  a\x00zc%def", 10));
    lexer->setCatcode(0, Token::CC_IGNORED);
    Token tokens[] = {
        Token(Token::TOK_SKIPPED, Token::CC_SPACE, " ", "  ", 0, 1, 0, 2),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "a", "a", 0, 1, 2, 3),
        Token(Token::TOK_SKIPPED, Token::CC_IGNORED,
                string("\0", 1), string("\0",1), 0, 1, 3, 4),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "z", "z", 0, 1, 4, 5),
        Token(Token::TOK_CHARACTER, Token::CC_LETTER, "c", "c", 0, 1, 5, 6),
        Token(Token::TOK_SKIPPED, Token::CC_COMMENT, "%", "%def", 0, 1, 6, 10),
    };
    check_output(tokens, sizeof(tokens)/sizeof(Token), run_lexer(lexer));
    }
}


