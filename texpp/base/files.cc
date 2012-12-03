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

#include <texpp/base/files.h>
#include <texpp/parser.h>
#include <texpp/logger.h>
#include <texpp/kpsewhich.h>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <fstream>
#include <sstream>

namespace texpp {
namespace base {

bool Openin::invoke(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);
    int stream = number->value(int(0));

    if(stream < 0 || stream > 15) {
        parser.logger()->log(Logger::ERROR,
                "Bad number (" + boost::lexical_cast<string>(stream) + ")",
                parser, parser.lastToken());
        stream = 0;
    }

    node->appendChild("equals", parser.parseOptionalEquals());

    Node::ptr fnameNode = parser.parseFileName();
    node->appendChild("file_name", fnameNode);

    string fname = fnameNode->value(string());
    string fullname = kpsewhich(fname, parser.workdir());
    //std::cout << "name: '" << fnameNode->value(string()) << "'\n";
    //std::cout << "fullname: '" << fullname << "'\n";

    shared_ptr<std::istream> istream(new std::ifstream(fullname.c_str()));
    if(!istream->fail()) {
        shared_ptr<Lexer> lexer(new Lexer(fullname, istream));
        parser.setSymbol("read" + boost::lexical_cast<string>(stream),
                                    InFile(lexer), true);
    } else {
        parser.setSymbol("read" + boost::lexical_cast<string>(stream),
                                    InFile(), true);
    }

    return true;
}

bool Closein::invoke(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);
    int stream = number->value(int(0));

    if(stream < 0 || stream > 15) {
        parser.logger()->log(Logger::ERROR,
                "Bad number (" + boost::lexical_cast<string>(stream) + ")",
                parser, parser.lastToken());
        stream = 0;
    }

    parser.setSymbol("read" + boost::lexical_cast<string>(stream),
                                InFile(), true);

    return true;
}

bool Read::invokeWithPrefixes(Parser& parser, shared_ptr<Node> node,
                                    std::set<string>& prefixes)
{
    bool global = checkPrefixes(parser, prefixes);
    prefixes.clear();

    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);
    int stream = number->value(int(0));

    static vector<string> kw_to(1, string("to"));
    Node::ptr to = parser.parseKeyword(kw_to);
    if(!to) {
        parser.logger()->log(Logger::ERROR,
                "Missing `to' inserted",
                parser, parser.lastToken());
        to = Node::ptr(new Node("keyword"));
    }

    node->appendChild("to", to);

    Node::ptr spaces(new Node("optional_spaces"));
    while(parser.peekToken(false) &&
            parser.peekToken(false)->isCharacterCat(Token::CC_SPACE)) {
        parser.nextToken(&spaces->tokens(), false);
    }
    node->appendChild("optional_spaces", spaces);

    Node::ptr tokenNode = parser.parseControlSequence(false);
    node->appendChild("token", tokenNode);

    Token::ptr ltoken = tokenNode->value(Token::ptr());

    // Get assosiated lexer
    InFile infile = 
        parser.symbol("read" + boost::lexical_cast<string>(stream), InFile());

    shared_ptr<Lexer> lexer = infile.lexer;
    if(!lexer) {
        if(parser.interaction() == Parser::NONSTOPMODE ||
                parser.interaction() == Parser::BATCHMODE) {
            parser.logger()->log(Logger::ERROR,
                    "Emergency stop",
                    parser, parser.lastToken());
            if(!parser.ignoreEmergency()) {
                parser.end();
                return true;
            } else {
                lexer = shared_ptr<Lexer>(new Lexer("",
                    shared_ptr<std::istream>(new std::istringstream(""))));
            }
        } else {
            // read from terminal
            //std::cin.sync();
            //lexer = shared_ptr<Lexer>(new Lexer("", NULL));
            //if(stream >= 0) {
            //    std::cout << ltoken->texRepr() << "=" << std::flush;
            //}
            // XXX: implement enforced batchmode
            lexer = shared_ptr<Lexer>(new Lexer("",
                shared_ptr<std::istream>(new std::istringstream(""))));
        }
    }

    // prepare the lexer
    lexer->setEndlinechar(parser.lexer()->endlinechar());
    for(int n=0; n<256; ++n) {
        lexer->setCatcode(n, parser.lexer()->catcode(n));
    }

    // read the tokens
    Token::list_ptr tokens(new Token::list());

    int level = 0;
    Token::ptr token;
    while(token = lexer->nextToken()) {
        if(!token->isSkipped()) {
            if(token->isCharacterCat(Token::CC_BGROUP))
                ++level;
            else if(token->isCharacterCat(Token::CC_EGROUP))
                --level;

            if(level >= 0)
                tokens->push_back(token->lcopy());
        }

        if(level <= 0 && token->isLastInLine())
            break;
    }

    if(!token) {
        // end of file
        int endlinechar = lexer->endlinechar();
        if(endlinechar >= 0 && endlinechar <= 255) {
            int cc = lexer->catcode(endlinechar);
            if(cc == Token::CC_EOL)
                tokens->push_back(Token::create(
                    Token::TOK_CONTROL, Token::CC_ESCAPE, "\\par"));
            else
                tokens->push_back(Token::create(
                    Token::TOK_CHARACTER, Token::CatCode(cc),
                    string(1, char(endlinechar))));
        }
        if(infile.lexer)
            parser.setSymbol("read" + boost::lexical_cast<string>(stream),
                            InFile(), true); 
    }

    // sync if reading from the terminal
    //if(!infile.lexer) {
    //    lexer.reset();
    //    std::cin.sync();
    //}

    parser.setSymbol(ltoken,
        Command::ptr(new UserMacro(ltoken ? ltoken->value() : "\\undefined",
            Token::list_ptr(new Token::list()), tokens, false, false)),
        global);

    return true;
}

bool Immediate::invokeWithPrefixes(Parser& parser, shared_ptr<Node>,
                                std::set<string>& prefixes)
{
    size_t immediate = prefixes.count(name());
    if(prefixes.size() != immediate) {
        parser.logger()->log(Logger::ERROR,
            "You can't use a prefix with `" +
            texRepr(&parser) + "'",
            parser, parser.lastToken());
    }
    prefixes.insert(name());
    return true;
}

bool Openout::invoke(Parser& parser, shared_ptr<Node> node)
{
    std::set<string> prefixes;
    return invokeWithPrefixes(parser, node, prefixes);
}

bool Openout::invokeWithPrefixes(Parser& parser,
                Node::ptr node, std::set<string>& prefixes)
{
    size_t immediate = prefixes.count("\\immediate");
    if(prefixes.size() != immediate) {
        parser.logger()->log(Logger::ERROR,
            "You can't use a prefix with `" +
            texRepr(&parser) + "'",
            parser, parser.lastToken());
    }

    prefixes.clear();

    if(!immediate) {
        parser.logger()->log(Logger::ERROR,
                "Openout without immediate is NOT supported",
                parser, parser.lastToken());
    }

    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);
    int stream = number->value(int(0));

    if(stream < 0 || stream > 15) {
        parser.logger()->log(Logger::ERROR,
                "Bad number (" + boost::lexical_cast<string>(stream) + ")",
                parser, parser.lastToken());
        stream = 0;
    }

    node->appendChild("equals", parser.parseOptionalEquals());

    Node::ptr fnameNode = parser.parseFileName();
    node->appendChild("file_name", fnameNode);

    string fname = kpseextend(fnameNode->value(string()));

    shared_ptr<std::ostream> ostream(new std::ofstream(fname.c_str()));
    if(!ostream->fail()) {
        parser.setSymbol("write" + boost::lexical_cast<string>(stream),
                                    OutFile(ostream), true);
        string msg = texRepr(&parser) + boost::lexical_cast<string>(stream)
                        + " = `" + fname + "'.\n\n";
        parser.logger()->log(Logger::MTRACING, msg,
                                parser, parser.lastToken());
    } else {
        parser.logger()->log(Logger::ERROR, "Emergency stop",
                                parser, parser.lastToken());
        if(!parser.ignoreEmergency())
            parser.end();
        //parser.setSymbol("write" + boost::lexical_cast<string>(stream),
        //                            OutFile(), true);
    }

    return true;
}

bool Closeout::invoke(Parser& parser, shared_ptr<Node> node)
{
    std::set<string> prefixes;
    return invokeWithPrefixes(parser, node, prefixes);
}

bool Closeout::invokeWithPrefixes(Parser& parser,
                Node::ptr node, std::set<string>& prefixes)
{
    size_t immediate = prefixes.count("\\immediate");
    if(prefixes.size() != immediate) {
        parser.logger()->log(Logger::ERROR,
            "You can't use a prefix with `" +
            texRepr(&parser) + "'",
            parser, parser.lastToken());
    }

    prefixes.clear();

    if(!immediate) {
        parser.logger()->log(Logger::ERROR,
                "Closeout without immediate is NOT supported",
                parser, parser.lastToken());
    }

    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);
    int stream = number->value(int(0));

    if(stream < 0 || stream > 15) {
        parser.logger()->log(Logger::ERROR,
                "Bad number (" + boost::lexical_cast<string>(stream) + ")",
                parser, parser.lastToken());
        stream = 0;
    }

    parser.setSymbol("write" + boost::lexical_cast<string>(stream),
                                OutFile(), true);

    return true;
}

bool Write::invoke(Parser& parser, shared_ptr<Node> node)
{
    std::set<string> prefixes;
    return invokeWithPrefixes(parser, node, prefixes);
}

bool Write::invokeWithPrefixes(Parser& parser,
                Node::ptr node, std::set<string>& prefixes)
{
    using boost::lexical_cast;

    size_t immediate = prefixes.count("\\immediate");
    if(prefixes.size() != immediate) {
        parser.logger()->log(Logger::ERROR,
            "You can't use a prefix with `" +
            texRepr(&parser) + "'",
            parser, parser.lastToken());
    }

    prefixes.clear();

    if(!immediate) {
        parser.logger()->log(Logger::ERROR,
                "Write without immediate is NOT supported",
                parser, parser.lastToken());
    }

    Node::ptr number = parser.parseNumber();
    node->appendChild("number", number);
    int stream = number->value(int(0));

    if(parser.symbol("tracingmacros", int(0)) >= 2) {
        // read the tokens without expanding to show them in the trace
        Node::ptr text = parser.parseGeneralText(false);
        Token::list_ptr tokens =
            text->child("balanced_text")->value(Token::list_ptr());

        parser.logger()->log(Logger::MTRACING,
                texRepr(&parser) + "->" +
                (tokens ? Token::texReprList(*tokens, &parser) : ""),
                parser, parser.lastToken());

        // push tokens back and re-read them with expansion turned on
        Node::ChildrenList::reverse_iterator rend = text->children().rend();
        for(Node::ChildrenList::reverse_iterator it =
                text->children().rbegin(); it != rend; ++it) {
            parser.pushBack(&(it->second->tokens()));
        }
    }

    // TODO: expand text later
    Parser::Mode prevMode = parser.mode();
    parser.setMode(Parser::NULLMODE);
    Node::ptr text = parser.parseGeneralText(true);
    parser.setMode(prevMode);

    node->appendChild("text", text);
    
    string str;
    Token::list_ptr tokens =
        text->child("balanced_text")->value(Token::list_ptr());

    if(tokens) {
        str = Token::texReprList(*tokens, &parser);
        /*
        Token::list tokens_show;
        BOOST_FOREACH(Token::ptr token, *tokens) {
            Command::ptr cmd = parser.symbol(token, Command::ptr());
            if(token->isControl() && !cmd) {
                parser.logger()->log(Logger::ERROR,
                    "Undefined control sequence", parser, token);
            } else {
                tokens_show.push_back(token);
            }
        }
        str = Token::texReprList(tokens_show, &parser);
        */
    }

    OutFile outfile = 
        parser.symbol("write" + boost::lexical_cast<string>(stream), OutFile());

    if(outfile.ostream) {
        (*outfile.ostream) << str << std::endl;
    } else {
        parser.logger()->log(Logger::WRITE, str, parser, parser.lastToken());
                //text->child("right_brace")->value(Token::ptr()));
    }

    return true;
}

bool Message::invoke(Parser& parser, Node::ptr node)
{
    using boost::lexical_cast;
    // TODO: expand text later

    Node::ptr text = parser.parseGeneralText(true);
    node->appendChild("text", text);
    
    string str;
    Token::list_ptr tokens =
        text->child("balanced_text")->value(Token::list_ptr());

    if(tokens) {
        str = Token::texReprList(*tokens, &parser);
        /*
        Token::list tokens_show;
        BOOST_FOREACH(Token::ptr token, *tokens) {
            Command::ptr cmd = parser.symbol(token, Command::ptr());
            if(token->isControl() && !cmd) {
                parser.logger()->log(Logger::ERROR,
                    "Undefined control sequence", parser, token);
            } else {
                tokens_show.push_back(token);
            }
        }
        str = Token::texReprList(tokens_show, &parser);
        */
    }

    parser.logger()->log(Logger::MESSAGE, str, parser, parser.lastToken());
                //text->child("right_brace")->value(Token::ptr()));
    return true;
}

bool Input::invoke(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr fnameNode = parser.parseFileName();
    node->appendChild("file_name", fnameNode);

    string fname = fnameNode->value(string());
    string fullname = kpsewhich(fname, parser.workdir());

    parser.input(fname, fullname);

    return true;
}

bool Endinput::invoke(Parser& parser, shared_ptr<Node>)
{
    parser.endinput();
    return true;
}

} // namespace base
} // namespace texpp

