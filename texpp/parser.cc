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

#include <texpp/parser.h>
#include <texpp/logger.h>

#include <texpp/base/base.h>
#include <texpp/base/show.h>
#include <texpp/base/variable.h>
#include <texpp/base/integer.h>
#include <texpp/base/dimen.h>
#include <texpp/base/glue.h>
#include <texpp/base/parshape.h>
#include <texpp/base/font.h>
#include <texpp/base/box.h>
#include <texpp/base/misc.h>
#include <texpp/base/files.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <climits>
#include <cassert>
#include <iterator>
#include <unistd.h>
#include <ctime>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

namespace {

const texpp::string modeNames[] = {
    "no",
    "vertical",
    "horizontal",
    "internal vertical",
    "restricted horizontal",
    "math",
    "display math",
    "unknown"
};
} // namespace

namespace texpp {

string Parser::BANNER = "This is TeXpp, Version 0.0";

using base::Dimen;

const string& Node::valueString() const
{
    static const string empty;
    if(m_value.type() != typeid(string)) return empty;
    else return *unsafe_any_cast<string>(&m_value);
}

Node::ptr Node::child(const string& name)
{
    ChildrenList::iterator end = m_children.end();
    for(ChildrenList::iterator it = m_children.begin(); it != end; ++it) {
        if(it->first == name) return it->second;
    }
    return Node::ptr();
}

Token::ptr Node::lastToken()
{
    ChildrenList::reverse_iterator rend = m_children.rend();
    for(ChildrenList::reverse_iterator it = m_children.rbegin();
                                            it != rend; ++it) {
        Token::ptr token = it->second->lastToken();
        if(token) return token;
    }

    Token::list::reverse_iterator rend1 = m_tokens.rend();
    for(Token::list::reverse_iterator it1 = m_tokens.rbegin();
                                            it1 != rend1; ++it1) {
        if(!(*it1)->isSkipped()) return *it1;
    }

    return Token::ptr();
}

string Node::repr() const
{
    return "Node(" + reprString(m_type)
        + (m_value.empty() ? "" : ", " + reprAny(m_value))
        + ")";
}

string Node::treeRepr(size_t indent) const
{
    string str = repr();
    if(!m_children.empty()) {
        str += ":\n";
        ChildrenList::const_iterator end = m_children.end();
        for(ChildrenList::const_iterator it = m_children.begin();
                                            it != end; ++it) {
            str += string(indent+2, ' ') +
                    it->first + ": " + it->second->treeRepr(indent+2);
        }
    } else {
        str += '\n';
    }
    return str;
}

string Node::source(const string& fileName) const
{
    string str;
    BOOST_FOREACH(Token::ptr token, m_tokens) {
        if(fileName.empty() || token->fileName() == fileName)
            str += token->source();
    }
    typedef pair<string, Node::ptr> C;
    BOOST_FOREACH(C c, m_children) {
        str += c.second->source(fileName);
    }
    return str;
}

unordered_map<shared_ptr<string>,string> Node::sources() const
{
    unordered_map<shared_ptr<string>,string> src;

    string* cur_str = 0;
    shared_ptr<string> cur_file;

    BOOST_FOREACH(Token::ptr token, m_tokens) {
        if(!cur_str || token->fileNamePtr() != cur_file) {
            cur_file = token->fileNamePtr();
            cur_str = &(src[cur_file]);
        }
        *cur_str += token->source();
    }
    typedef pair<string, Node::ptr> C;
    BOOST_FOREACH(C c, m_children) {
        unordered_map<shared_ptr<string>,string> sub_src(c.second->sources());
        unordered_map<shared_ptr<string>,string>::iterator end=sub_src.end();
        for(unordered_map<shared_ptr<string>, string>::iterator it =
                    sub_src.begin(); it != end; ++it) {
            src[it->first] += it->second;
        }
    }
    return src;
}

std::set<shared_ptr<string> > Node::files() const
{
    std::set<shared_ptr<string> > f;
    BOOST_FOREACH(Token::ptr token, m_tokens) {
        f.insert(token->fileNamePtr());
    }
    typedef pair<string, Node::ptr> C;
    BOOST_FOREACH(C c, m_children) {
        std::set<shared_ptr<string> > sub_f = c.second->files();
        f.insert(sub_f.begin(), sub_f.end());
    }
    return f;
}

shared_ptr<string> Node::oneFile() const
{
    std::set<shared_ptr<string> > f = this->files();
    if(f.size() == 1) return *f.begin();
    else return shared_ptr<string>();
}

bool Node::isOneFile() const
{
    shared_ptr<string> cur_file;
    BOOST_FOREACH(Token::ptr token, m_tokens) {
        if(!cur_file) {
            if(!token->fileNamePtr())
                return false;
            cur_file = token->fileNamePtr();
        } else if(cur_file != token->fileNamePtr()) {
            return false;
        }
    }
    typedef pair<string, Node::ptr> C;
    BOOST_FOREACH(C c, m_children) {
        if(!c.second->isOneFile())
            return false;
    }
    return true;
}

std::pair<size_t, size_t> Node::sourcePos() const
{
    std::pair<size_t, size_t> pos(Token::npos, Token::npos);
    BOOST_FOREACH(Token::ptr token, m_tokens) {
        if(token->lineNo() != 0) {
            if(pos.first == Token::npos)
                pos.first = token->linePos() + token->charPos();
            pos.second = token->linePos() + token->charEnd();
        }
    }
    typedef pair<string, Node::ptr> C;
    BOOST_FOREACH(C c, m_children) {
        std::pair<size_t, size_t> sub_pos = c.second->sourcePos();
        if(sub_pos.first != Token::npos) {
            if(pos.first == Token::npos)
                pos.first = sub_pos.first;
            pos.second = sub_pos.second;
        }
    }
    return pos;
}

Parser::Parser(const string& fileName, std::istream* file,
        const string& workdir, bool interactive, bool ignoreEmergency,
        shared_ptr<Logger> logger)
    : m_workdir(workdir), m_ignoreEmergency(ignoreEmergency),
      m_logger(logger), m_groupLevel(0),
      m_end(false), m_endinput(false), m_endinputNow(false),
      m_lineNo(1), m_mode(NULLMODE), m_prevMode(NULLMODE),
      m_hasOutput(false), m_currentGroupType(GROUP_DOCUMENT),
      m_customGroupBegin(false), m_customGroupEnd(false),
      m_interaction(ERRORSTOPMODE)
{
    m_lexer = shared_ptr<Lexer>(new Lexer(fileName, file, interactive, true));
    init();
}

Parser::Parser(const string& fileName, shared_ptr<std::istream> file,
        const string& workdir, bool interactive, bool ignoreEmergency,
        shared_ptr<Logger> logger)
    : m_workdir(workdir), m_ignoreEmergency(ignoreEmergency),
      m_logger(logger), m_groupLevel(0),
      m_end(false), m_endinput(false), m_endinputNow(false),
      m_lineNo(1), m_mode(NULLMODE), m_prevMode(NULLMODE),
      m_hasOutput(false), m_currentGroupType(GROUP_DOCUMENT),
      m_customGroupBegin(false), m_customGroupEnd(false),
      m_interaction(ERRORSTOPMODE)
{
    m_lexer = shared_ptr<Lexer>(new Lexer(fileName, file, interactive, true));
    init();
}

void Parser::init()
{
    if(!m_logger)
        m_logger = lexer()->interactive() ?
                        shared_ptr<Logger>(new ConsoleLogger) :
                        shared_ptr<Logger>(new NullLogger);

    base::initSymbols(*this);
    
    string banner = BANNER;
    if(!lexer()->interactive()) {
        char t[256];
        time_t tt = std::time(NULL);
        std::strftime(t, sizeof(t), " %e %b %Y %H:%M", std::localtime(&tt));
        string ts(t);
        boost::algorithm::to_upper(ts);
        banner += ts;
    }
    m_logger->log(Logger::WRITE, banner, *this, Token::ptr());
}

const string& Parser::modeName() const
{
    if(m_mode > DMATH)
        return modeNames[DMATH+1];
    return modeNames[m_mode];
}

any Parser::EMPTY_ANY;

const any& Parser::symbolAny(const string& name) const
{
    SymbolTable::const_iterator it = m_symbols.find(name);
    if(it != m_symbols.end())
        return it->second.second;
    return EMPTY_ANY;
}

void Parser::setSymbol(const string& name, const any& value, bool global)
{
    SymbolTable::iterator it = m_symbols.insert(
        std::make_pair(name, std::make_pair(0, any()))).first;

    if(!global && it->second.first != m_groupLevel) {
        m_symbolsStack.push_back(std::make_pair(name, it->second));
        it->second.first = m_groupLevel;
    } else if(global && it->second.first >= 0) {
        it->second.first = -1;
    }
    it->second.second = value;
    setSpecialSymbol(name, value);
}

void Parser::setSymbolDefault(const string& name, const any& defaultValue)
{
    pair<SymbolTable::iterator, bool> p = m_symbols.insert(
        std::make_pair(name, std::make_pair(0, any())));

    if(p.second) // new item
        p.first->second = std::make_pair(0, defaultValue);
}

void Parser::setSpecialSymbol(const string& name, const any& value)
{
    if(value.type() == typeid(int)) {
        if(name == "endlinechar") {
            m_lexer->setEndlinechar(*unsafe_any_cast<int>(&value));
        } else if(name.substr(0, 7) == "catcode") {
            std::istringstream s(name.substr(7));
            int n = 0; s >> n;
            if(!s.fail() && n >= 0 && n <= 255) {
                m_lexer->setCatcode(n, *unsafe_any_cast<int>(&value));
            }
        }
    }
}

void Parser::beginGroup()
{
    m_symbolsStackLevels.push_back(m_symbolsStack.size());
    m_aftergroupTokensStack.push_back(Token::list());
    ++m_groupLevel;
}

void Parser::endGroup()
{
    if(m_groupLevel <= 0) {
        m_logger->log(Logger::CRITICAL,
            "Something really wrong: group level have reached zero!",
            *this, lastToken());
        return;
    }
    //assert(m_groupLevel > 0); //XXX!
    size_t symbolsStackLevels = m_symbolsStackLevels.empty() ? 0 :
                                m_symbolsStackLevels.back();
    while(m_symbolsStack.size() > symbolsStackLevels) {
        SymbolStack::reference item = m_symbolsStack.back();
        SymbolTable::iterator it = m_symbols.find(item.first);

        int l = it->second.first;

        if(l >= 0) {
            it->second = item.second;
            setSpecialSymbol(it->first, it->second.second);
        }

        if(symbol("tracingrestores", int(0)) > 0) {
            string str;
            any value;

            if(l >= 0) {
                str = "restoring ";
                value = item.second.second;
            } else {
                str = "retaining ";
                value = it->second.second;
            }

            string escape = escapestr();
            if(item.first == "font")
                str += "current font";
            else if(item.first.size() > 0 && item.first[0] == '\\')
                str += escape + item.first.substr(1);
            else if(item.first.size() > 0 && item.first[0] == '`')
                str += item.first.substr(1);
            else
                str += escape + item.first;

            str += "=";

            if(value.empty()) {
                str += "undefined";
            } else if(value.type() == typeid(Command::ptr)) {
                Command::ptr cmd = *unsafe_any_cast<Command::ptr>(&value);
                shared_ptr<base::UserMacro> m =
                    dynamic_pointer_cast<base::UserMacro >(cmd);
                if(m) {
                    str += m->texRepr(this, false, 38);
                            //60 > str.length() ? 60-str.length() : 1);
                } else {
                    str += (cmd ? cmd->texRepr(this) : "undefined");
                }
                //std::remove_copy(r.begin(), r.end(),
                //        std::back_inserter(str), '\n');
            } else if(value.type() == typeid(base::ParshapeInfo)) {
                str += boost::lexical_cast<string>(
                    unsafe_any_cast<base::ParshapeInfo>(&value)
                                        ->parshape.size());
            } else if(value.type() == typeid(shared_ptr<base::FontInfo>)) {
                shared_ptr<base::FontInfo> f =
                    *unsafe_any_cast<shared_ptr<base::FontInfo> >(&value);
                string fname = f ? f->selector : "";
                if(!fname.empty() && fname[0] == '\\') {
                    fname = escape + fname.substr(1);
                } else {
                    fname = escape + "FONT" + str;
                }
                str += fname;
            } else if(value.type() == typeid(base::Box)) {
                base::Box box = *unsafe_any_cast<base::Box>(&value);
                if(box.value) {
                    string w = base::InternalDimen::dimenToString(box.width);
                    string h = base::InternalDimen::dimenToString(box.height);
                    string s = base::InternalDimen::dimenToString(box.skip);
                    str += "\n" + escape +
                        (box.mode == RHORIZONTAL ? "hbox" : "vbox") +
                        "(" + w.substr(0, w.size()-2) +
                        "+" + s.substr(0, s.size()-2) +
                        ")x" + h.substr(0, h.size()-2);
                    if(box.mode == RVERTICAL)
                        str += " []";
                } else {
                    str += "void";
                }
            } else if(value.type() == typeid(Token::list)) {
                str += Token::texReprList(
                        *unsafe_any_cast<Token::list>(&value), this, false, 34);
                        //false, 70 > str.length() ? 70-str.length() : 1);
            } else {
                str += reprAny(value);
            }

            //if(str.size() > 72) {
            //   str = str.substr(0, 72-5) + "\\ETC.";
            //}

            logger()->log(Logger::TRACING,
                str, *this, Token::ptr());
        }

        m_symbolsStack.pop_back();
    }

    pushBack(&m_aftergroupTokensStack.back());

    m_aftergroupTokensStack.pop_back();
    m_symbolsStackLevels.pop_back();
    --m_groupLevel;
}

Node::ptr Parser::rawExpandToken(Token::ptr token)
{
    if(m_lockToken) {
        if(token->type() == m_lockToken->type() &&
                token->catCode() == m_lockToken->catCode() &&
                token->value() == m_lockToken->value())
            return Node::ptr();
    }

    Command::ptr cmd = symbol(token, Command::ptr());
    Macro::ptr macro = dynamic_pointer_cast<Macro>(cmd);

    if(cmd && !macro)
        return Node::ptr();

    Node::ptr node(new Node("macro"));
    Node::ptr child(new Node("control_token"));
    child->tokens().push_back(token);
    child->setValue(token);
    node->appendChild("control_sequence", child);
    bool expanded = true;
    
    pushBack(NULL);

    if(m_conditionals.empty() || m_conditionals.back().active)
        traceCommand(token, true);

    if(!cmd) {
        logger()->log(Logger::ERROR,
            "Undefined control sequence", *this, token);
        /*token = Token::create(Token::TOK_SKIPPED,
                    token->catCode(), token->value(), token->source(),
                    token->listPos(), token->lineNo(),
                    token->charPos(), token->charEnd(),
                    token->isLastInLine(), token->fileNamePtr());*/
        //token = token->lcopy();
        //token->setType(Token::TOK_SKIPPED);
        //node->setValue(Token::list(1, token));
        node->setType("undefined_control_sequence");

    } else if(dynamic_pointer_cast<ConditionalBegin>(macro)) {
        ConditionalBegin::ptr condBegin =
            static_pointer_cast<ConditionalBegin>(macro);

        ConditionalInfo cinfo0;
        cinfo0.parsed = false;
        cinfo0.ifcase = false;
        cinfo0.active = true;
        cinfo0.value = true;
        cinfo0.branch = 0;

        m_conditionals.push_back(cinfo0);
        size_t level = m_conditionals.size();

        // At this point the rawNextToken may be called recursively
        m_commandStack.push_back(condBegin);
        condBegin->evaluate(*this, node);
        m_commandStack.pop_back();

        pushBack(NULL);

        assert(m_conditionals.size() >= level);
        ConditionalInfo& cinfo = m_conditionals[level-1];

        cinfo.ifcase = node->valueAny().type() == typeid(int);
        if(cinfo.ifcase) {
            cinfo.value = node->value(int(0));
            cinfo.active = cinfo.value == 0;
        } else {
            cinfo.value = node->value(bool(false));
            cinfo.active = cinfo.value;
        }

        cinfo.branch = 0;
        cinfo.parsed = true;

        if(symbol("tracingcommands", int(0)) > 1/* && mode() != NULLMODE*/) {
            string str;
            if(cinfo.ifcase) {
                str = "case " +
                    boost::lexical_cast<string>(cinfo.value);
            } else {
                str = cinfo.value ? "true" : "false";
            }
            logger()->log(Logger::TRACING, str, *this, token);
        }

        //m_conditionals.push_back(cinfo);

        if(!cinfo.active) {
            node->appendChild("false_conditional",
                    parseFalseConditional(level, true, cinfo.ifcase));
            pushBack(NULL);
        }

    } else if(dynamic_pointer_cast<ConditionalOr>(macro)) {
        if(!m_conditionals.empty() && !m_conditionals.back().parsed) {
            node->setValue(Token::list_ptr(
                new Token::list(1, Token::create(
                token->type(), token->catCode(), token->value(),
                "", token->linePos(), token->lineNo(),
                token->charEnd(), token->charEnd(),
                token->isLastInLine(), token->fileNamePtr()))));
            expanded = false;
        } else if((m_conditionals.empty() ||
                m_conditionals.back().branch < 0 ||
                !m_conditionals.back().ifcase)) {
            logger()->log(Logger::ERROR,
                "Extra " + macro->texRepr(this), *this, token);
        } else {
            ConditionalInfo& cinfo = m_conditionals.back();
            ++cinfo.branch;
            cinfo.active = (cinfo.value == cinfo.branch);
            if(!cinfo.active) {
                node->appendChild("false_conditional",
                    parseFalseConditional(
                        m_conditionals.size(), true, true));
                pushBack(NULL);
            }
        }
    } else if(dynamic_pointer_cast<ConditionalElse>(macro)) {
        if(!m_conditionals.empty() && !m_conditionals.back().parsed) {
            node->setValue(Token::list_ptr(
                new Token::list(1, Token::create(
                token->type(), token->catCode(), token->value(),
                "", token->linePos(), token->lineNo(),
                token->charEnd(), token->charEnd(),
                token->isLastInLine(), token->fileNamePtr()))));
            expanded = false;
        } else if((m_conditionals.empty() ||
                m_conditionals.back().branch < 0)) {
            logger()->log(Logger::ERROR,
                "Extra " + macro->texRepr(this), *this, token);
        } else {
            ConditionalInfo& cinfo = m_conditionals.back();
            if(cinfo.ifcase) {
                cinfo.active = cinfo.value < 0 ||
                                cinfo.value > cinfo.branch;
            } else {
                cinfo.active = !cinfo.value;
            }
            cinfo.branch = -1;
            if(!cinfo.active) {
                node->appendChild("false_conditional",
                    parseFalseConditional(
                        m_conditionals.size(), false, false));
                pushBack(NULL);
            }
        }
    } else if(dynamic_pointer_cast<ConditionalEnd>(macro)) {
        if(!m_conditionals.empty() && !m_conditionals.back().parsed) {
            node->setValue(Token::list_ptr(
                new Token::list(1, Token::create(
                token->type(), token->catCode(), token->value(),
                "", token->linePos(), token->lineNo(),
                token->charEnd(), token->charEnd(),
                token->isLastInLine(), token->fileNamePtr()))));
            expanded = false;
        } else if(m_conditionals.empty()) {
            logger()->log(Logger::ERROR,
                "Extra " + macro->texRepr(this), *this, token);
        } else {
            m_conditionals.pop_back();
        }

    } else {
        // At this point the rawNextToken may be called recursively
        m_commandStack.push_back(macro);
        macro->expand(*this, node);
        m_commandStack.pop_back();
        pushBack(NULL);

    }

    // TODO: the next lines is horible. Either Node::value should return
    //       a reference or the value itself should be Token::list_ptr.
    Token::list_ptr newTokens = node->value(Token::list_ptr());
    if(!newTokens) newTokens = Token::list_ptr(new Token::list());
    newTokens->insert(newTokens->begin(),
                Token::create(
                    expanded ? Token::TOK_SKIPPED : token->type(),
                    token->catCode(), token->value(), node->source(),
                    0, 0, 0, 0,
                    false, lexer()->fileNamePtr())
            );
#warning XXX node can consists from tokens from several files!
    node->setValue(newTokens);

    return node;
}

Token::ptr Parser::rawNextToken(bool expand)
{
    Token::ptr token;

    while(true) {
        if(!m_tokenQueue.empty()) {
            token = m_tokenQueue.front();
            m_tokenQueue.pop_front();
        } else {
            if(m_endinputNow) {
                endinputNow();
                continue;
            }

            token = m_lexer->nextToken();
            if(token && !token->isSkipped())
                m_lastToken = token;

            if(!m_inputStack.empty()) {
                if(!token) {
                    endinputNow();
                    continue;
                } else if(m_endinput && token->isLastInLine()) {
                    m_endinputNow = true;
                }
            }
        }
        break;
    }

    if(token && token->isControl() && expand &&
                m_noexpandTokens.count(token) == 0) {
        Node::ptr node = rawExpandToken(token);
        if(node) {
            Token::list_ptr newTokens = node->value(Token::list_ptr());
            assert(newTokens && !newTokens->empty());

            Token::list::reverse_iterator rend = newTokens->rend();
            Token::list::reverse_iterator it = newTokens->rbegin();
            for(; it+1 != rend; ++it) {
                assert((*it)->source().empty());
                m_tokenQueue.push_front(*it);
            }
            if(it != rend)
                token = *it;
        }
    }

    return token;
}

Token::ptr Parser::nextToken(vector< Token::ptr >* tokens, bool expand)
{
    if(m_tokenSource.empty())
        peekToken(expand);

    if(tokens) {
        tokens->insert(tokens->end(),
                m_tokenSource.begin(), m_tokenSource.end());
    }

    Token::ptr token = m_token;
    if(!m_lexer->interactive() && m_lexer->lineNo() != m_lineNo) {
        m_lineNo = m_lexer->lineNo();
        setSymbol("inputlineno", int(m_lineNo), true);
    }

    m_tokenSource.clear();
    m_token.reset();

    return token;

#if 0
    if(m_end) {
        if(!m_lexer->interactive()) {
            // Return the rest of the document as skipped tokens
            Token::ptr token;
            while(token = rawNextToken(false)) {
                token->setType(Token::TOK_SKIPPED);
                if(tokens) tokens->push_back(token);
            }
        }
        return Token::ptr();
    }

    m_token.reset();

    // skip ignored tokens
    Token::ptr token = rawNextToken(expand);
    while(token && token->isSkipped()) {
        if(token->catCode() == Token::CC_INVALID) {
            m_logger->log(Logger::ERROR,
                "Text line contains an invalid character", *this, token);
        }

        if(tokens) tokens->push_back(token);
        token = rawNextToken(expand);
    }

    // real token
    if(tokens && token) tokens->push_back(token);
    Token::ptr ret = token;
    if(token && token->lineNo())
        m_lastToken = token;

    // skip ignored tokens until EOL
    if(token && !token->isLastInLine()) {
        while(true) {
            token = rawNextToken(false);
            if(!token) {
                break;
            } else if(!token->isSkipped()) {
                //m_token = token;
                m_tokenQueue.push_front(token);
                break;
            }

            if(token->catCode() == Token::CC_INVALID) {
                m_logger->log(Logger::ERROR,
                    "Text line contains an invalid character", *this, token);
            }

            if(tokens) tokens->push_back(token);
            
            if(token->isLastInLine()) {
                break;
            }
        }
    }

    if(!m_lexer->interactive() && m_lexer->lineNo() != m_lineNo) {
        m_lineNo = m_lexer->lineNo();
        setSymbol("inputlineno", int(m_lineNo), true);
    }
    return ret;
#endif
}

Token::ptr Parser::lastToken()
{
    return m_lastToken;
}

Token::ptr Parser::peekToken(bool expand)
{
    //int n = 1; // XXX
    if(m_end) {
        m_token.reset();
        if(!m_lexer->interactive()) {
            // Return the rest of the document as skipped tokens
            Token::ptr token;
            while(token = rawNextToken(false)) {
                token->setType(Token::TOK_SKIPPED);
                m_tokenSource.push_back(token);
            }
        }
        return Token::ptr();
    }

    // check for cached token
    if(!m_tokenSource.empty())
        return m_token;

    // skipped tokens
    Token::ptr token;
    Token::list tokenSource;
    while((token = rawNextToken(expand)) && token->isSkipped()) {
        if(token->catCode() == Token::CC_INVALID) {
            m_logger->log(Logger::ERROR,
                "Text line contains an invalid character", *this, token);
        }
        
        tokenSource.push_back(token);
    }

    // real token
    Token::ptr mtoken = token;
    if(token) {
        //if(token->lineNo())
        //    m_lastToken = token;
        tokenSource.push_back(token);
    }

    // XXX
    /*
    // skipped tokens until EOL
    if(token && !token->isLastInLine()) {
        while(token = rawNextToken(false)) {
            if(!token->isSkipped() || !token->lineNo()) {
                m_tokenQueue.push_front(token);
                break;
            }

            if(token->catCode() == Token::CC_INVALID) {
                m_logger->log(Logger::ERROR,
                    "Text line contains an invalid character", *this, token);
            }

            tokenSource.push_back(token);
            
            if(token->isLastInLine()) {
                break;
            }
        }
    }
    */

    pushBack(NULL); // peekToken may be called recursively

    m_token = mtoken;
    m_tokenSource = tokenSource;

    return m_token;

    #if 0
    // check the queue
    std::deque<Token::ptr >::iterator end = m_tokenQueue.end();
    for(std::deque<Token::ptr >::iterator it = m_tokenQueue.begin();
                                                 it != end; ++it) {
        if(!(*it)->isSkipped()) {
            m_lastToken = m_token = *it;
            return m_token;
            /*if(!m_token) m_token = *it;
            if(!--n) return *it;*/
        }
    }

    // read tokens from input
    while(true) {
        Token::ptr token = m_lexer->nextToken();
        if(!token) break;

        m_tokenQueue.push_back(token);
        if(!token->isSkipped()) {
            m_lastToken = m_token = token;
            return m_token;
            /*if(!m_token) m_token = token;
            if(!--n) return token;*/
        }
    }

    return Token::ptr();
    #endif
}

void Parser::pushBack(vector< Token::ptr >* tokens)
{
    std::copy(m_tokenSource.rbegin(), m_tokenSource.rend(),
                std::front_inserter(m_tokenQueue));

    m_tokenSource.clear();
    m_token.reset();

    if(tokens) {
        std::copy(tokens->rbegin(), tokens->rend(),
                    std::front_inserter(m_tokenQueue));
    }
    // NOTE: lastToken is NOT changed
}

void Parser::input(const string& fileName, const string& fullName)
{
    // TODO: stop scaning genericText on file boundary
    // (for example \def\x{...} can't be spread across several files
    shared_ptr<std::istream> istream(new std::ifstream(fullName.c_str()));
    if(istream->fail()) {
        logger()->log(Logger::ERROR,
            "I can't find file `" + fileName + "'",
            *this, lastToken());

        logger()->log(Logger::ERROR,
            "Emergency stop",
            *this, lastToken());

        if(!ignoreEmergency())
            end();

        return;
    }

    m_inputStack.push_back(std::make_pair(m_lexer, m_tokenQueue));

    shared_ptr<Lexer> lexer(new Lexer(fullName, istream, false, true));
    lexer->setEndlinechar(m_lexer->endlinechar());
    for(int n=0; n<256; ++n) {
        lexer->setCatcode(n, m_lexer->catcode(n));
    }

    m_lexer = lexer;
    m_tokenQueue.clear();

    logger()->log(Logger::MESSAGE, "(" + fullName, *this, lastToken());
}

void Parser::endinputNow()
{
    if(m_inputStack.empty())
        return;
    m_lexer = m_inputStack.back().first;
    m_tokenQueue = m_inputStack.back().second;
    m_inputStack.pop_back();
    m_endinput = false;
    m_endinputNow = false;
    logger()->log(Logger::PLAIN, ")", *this, lastToken());
}

void Parser::processTextCharacter(char ch, Token::ptr token)
{
    if(mode() != RHORIZONTAL)
        setMode(HORIZONTAL);

    if(m_prevMode != m_mode)
        traceCommand(token);

    m_hasOutput = true;

    int prevSpacefactor = symbol("spacefactor", true);
    int spacefactor = symbol(
        "sfcode" + boost::lexical_cast<string>(int(ch)), int(0));

    if(spacefactor != 0) {
        if(prevSpacefactor > 1000 && spacefactor < 1000)
            spacefactor = 1000;
        setSymbol("spacefactor", spacefactor, true);
    }
}

void Parser::resetParagraphIndent()
{
    if(symbol("parshape", base::ParshapeInfo()).parshape.size() != 0)
        setSymbol("parshape", base::ParshapeInfo());

    if(symbol("hangindent", Dimen(0)).value != 0)
        setSymbol("hangindent", Dimen(0));

    if(symbol("hangafter", int(0)) != 1)
        setSymbol("hangafter", int(1));

    if(symbol("looseness", int(0)) != 0)
        setSymbol("looseness", int(0));

    if(symbol("spacefactor", int(0)) != 1000)
        setSymbol("spacefactor", int(1000), true);
}

bool Parser::helperIsImplicitCharacter(Token::CatCode catCode, bool expand)
{
    if(peekToken(expand)) {
        if(peekToken(expand)->isCharacterCat(catCode)) {
            return true;
        } else if(peekToken(expand)->isControl()) {
            shared_ptr<TokenCommand> c =
                symbolCommand<TokenCommand>(peekToken(expand));
            if(c && c->token()->isCharacterCat(catCode))
                return true;
        }
    }
    return false;
}

Node::ptr Parser::parseFalseConditional(size_t level, bool sElse, bool sOr)
{
    Node::ptr node(new Node("skipped_conditional"));

    Token::ptr token;
    while((token = peekToken(false)) && m_conditionals.size() >= level) {
        Command::ptr cmd = symbol(token, Command::ptr());
        nextToken(&node->tokens(), false);
        
        if(dynamic_pointer_cast<ConditionalBegin>(cmd)) {
            ConditionalInfo cinfo;
            cinfo.parsed = false;
            cinfo.active = false;
            m_conditionals.push_back(cinfo);

        } else if(dynamic_pointer_cast<ConditionalOr>(cmd)) {
            if(sOr && m_conditionals.size() == level) {
                ConditionalInfo& cinfo = m_conditionals.back();
                ++cinfo.branch;
                cinfo.active = (cinfo.value == cinfo.branch);
                if(cinfo.active)
                    return node;
            }

        } else if(dynamic_pointer_cast<ConditionalElse>(cmd)) {
            if(sElse && m_conditionals.size() == level) {
                ConditionalInfo& cinfo = m_conditionals.back();
                if(cinfo.ifcase) {
                    cinfo.active = cinfo.value < 0 ||
                                    cinfo.value > cinfo.branch;
                } else {
                    cinfo.active = !cinfo.value;
                }
                cinfo.branch = -1;
                sElse = sOr = false;
                if(cinfo.active)
                    return node;
            }

        } else if(dynamic_pointer_cast<ConditionalEnd>(cmd)) {
            if(m_conditionals.size() == level) {
                m_conditionals.pop_back();
                return node;
            }
            m_conditionals.pop_back();
        }
    }

    // TODO: error
    return node;
}

Node::ptr Parser::parseCommand(Command::ptr command)
{
    Node::ptr node(new Node("command"));

    if(dynamic_pointer_cast<base::Prefix>(command)) {
        std::set<string> prefixes;
        Token::ptr token;
        while(peekToken()) {
            token = peekToken();

            command = symbol(token, Command::ptr());
            if(!command) break;

            int lastChildNumber = node->childrenCount();
            node->appendChild("prefix", parseControlSequence());

            m_commandStack.push_back(command);
            bool r = command->invokeWithPrefixes(*this, node, prefixes);
            m_commandStack.pop_back();

            if(!r) {
                pushBack(&node->children().back().second->tokens());
                node->children().pop_back();
                break;
            } else if(prefixes.empty()) {
                node->children()[lastChildNumber].first = "control_sequence";
                resetNoexpand();

                if(m_afterassignmentToken &&
                        dynamic_pointer_cast<base::Assignment>(command)) {
                    Token::list tokens(1, m_afterassignmentToken);
                    pushBack(&tokens);
                    m_afterassignmentToken.reset();
                }

                return node;
            }
        }
        logger()->log(Logger::ERROR,
            "You can't use a prefix with `" +
            token->meaning(this) + "'",
            *this, lastToken());
    } else {
        node->appendChild("control_sequence", parseControlSequence());

        m_commandStack.push_back(command);
        command->invoke(*this, node); // XXX check errors
        m_commandStack.pop_back();

        if(m_afterassignmentToken &&
                dynamic_pointer_cast<base::Assignment>(command)) {
            Token::list tokens(1, m_afterassignmentToken);
            pushBack(&tokens);
            m_afterassignmentToken.reset();
        }
    }

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseToken(bool expand)
{
    Node::ptr node(new Node("token"));
    Token::ptr token = peekToken(expand);

    if(token) {
        node->setValue(nextToken(&node->tokens(), expand));
    } else {
        logger()->log(Logger::ERROR, "Missing token inserted", *this, token);
        node->setValue(Token::create(Token::TOK_CONTROL,
                            Token::CC_ESCAPE, "inaccessible"));
    }

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseDMathToken()
{
    Node::ptr node(new Node("mmath_token"));
    nextToken(&node->tokens());

    if(!helperIsImplicitCharacter(Token::CC_MATHSHIFT, false)) {
        logger()->log(Logger::ERROR,
            "Display math should end with $$", *this, lastToken());
    } else {
        nextToken(&node->tokens());
    }

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseControlSequence(bool expand)
{
    Node::ptr node(new Node("control_sequence"));
    Token::ptr token = peekToken(expand);

    if(token && token->isControl()) {
        node->setValue(nextToken(&node->tokens(), expand));
    } else {
        logger()->log(Logger::ERROR,
            "Missing control sequence inserted", *this, lastToken());
        node->setValue(Token::create(Token::TOK_CONTROL,
                            Token::CC_ESCAPE, "inaccessible"));
    }

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseTextCharacter()
{
    Node::ptr node(new Node(peekToken() &&
        peekToken()->isCharacterCat(Token::CC_SPACE) ?
        "text_space" : "text_character" ));
    if(peekToken() && peekToken()->isCharacter()) {
        if(mode() != MATH && mode() != DMATH)
            processTextCharacter(peekToken()->value()[0], peekToken());
        node->setValue(peekToken()->value());
        nextToken(&node->tokens());
    } else {
        logger()->log(Logger::ERROR,
            "Missing character inserted", *this, lastToken());
        node->setValue(string(""));
    }

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseOptionalSpaces()
{
    Node::ptr node(new Node("optional_spaces"));
    while(helperIsImplicitCharacter(Token::CC_SPACE))
        nextToken(&node->tokens());
    return node;
}

Node::ptr Parser::parseKeyword(const vector<string>& keywords)
{
    Node::ptr node(new Node("keyword"));

    while(helperIsImplicitCharacter(Token::CC_SPACE))
        nextToken(&node->tokens());

    string value;
    vector<string>::const_iterator kwEnd = keywords.end();
    for(size_t n=1; peekToken() && peekToken()->isCharacter(); ++n) {
        value += std::tolower(peekToken()->value()[0]);
        nextToken(&node->tokens());

        vector<string>::const_iterator kw = keywords.begin();
        for(; kw != kwEnd; ++kw) {
            if(kw->substr(0, n) == value) break;
        }

        if(kw == kwEnd) {
            break;
        } else if(kw->size() == n) {
            node->setValue(value);
            resetNoexpand();
            return node;
        }
    }

    pushBack(&node->tokens());

    resetNoexpand();
    return Node::ptr();
}

Node::ptr Parser::parseOptionalKeyword(const vector<string>& keywords)
{
    Node::ptr node = parseKeyword(keywords);
    if(!node) {
        node = Node::ptr(new Node("keyword"));
        while(helperIsImplicitCharacter(Token::CC_SPACE))
            nextToken(&node->tokens());
        node->setValue(string());
        resetNoexpand();
    }
    return node;
}

Node::ptr Parser::parseOptionalEquals()
{
    Node::ptr node(new Node("optional_equals"));
    while(helperIsImplicitCharacter(Token::CC_SPACE))
        nextToken(&node->tokens());

    if(peekToken() && peekToken()->isCharacter('=', Token::CC_OTHER)) {
        node->setValue(nextToken(&node->tokens()));
    }

    /*
    if(oneSpaceAfter && helperIsImplicitCharacter(Token::CC_SPACE))
        nextToken(&node->tokens());
    */

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseOptionalSigns()
{
    Node::ptr node(new Node("optional_signs"));
    node->setValue(int(1));

    while(peekToken() && (
                helperIsImplicitCharacter(Token::CC_SPACE) ||
                peekToken()->isCharacter('+', Token::CC_OTHER) ||
                peekToken()->isCharacter('-', Token::CC_OTHER))) {
        if(peekToken()->isCharacter('-', Token::CC_OTHER)) {
            node->setValue(- node->value(int(0)));
        }
        nextToken(&node->tokens());
    }

    return node;
}

Node::ptr Parser::parseNormalInteger()
{

    Node::ptr node(new Node("normal_integer"));
    if(!peekToken()) {
        logger()->log(Logger::ERROR,
            "Missing number, treated as zero", *this, Token::ptr());
        node->setValue(int(0));
        resetNoexpand();
        return node;
    }

    Node::ptr integer =
        base::Variable::tryParseVariableValue<base::InternalInteger>(*this);
    if(integer) {
        node->appendChild("internal_integer", integer);
        node->setValue(integer->valueAny());
        resetNoexpand();
        return node;
    }

    if(peekToken()->isCharacter('`', Token::CC_OTHER)) {
        nextToken(&node->tokens());
        if(peekToken(false) && peekToken(false)->isCharacter()) {
            node->setValue(int((unsigned char) peekToken(false)->value()[0]));
            nextToken(&node->tokens(), false);
        } else if(peekToken(false) && peekToken(false)->isControl() &&
                    peekToken()->value().size() == 2) {
            node->setValue(int((unsigned char) peekToken(false)->value()[1]));
            nextToken(&node->tokens(), false);
        } else {
            logger()->log(Logger::ERROR,
                "Improper alphabetic constant", *this, lastToken());
            node->setValue(int(48)); // XXX: why 48 ?
        }
        if(helperIsImplicitCharacter(Token::CC_SPACE))
            nextToken(&node->tokens());

        resetNoexpand();
        return node;
    }
    
    int result = 0;
    int digits = 0;

    if(peekToken()->isCharacter('\"', Token::CC_OTHER)) {
        nextToken(&node->tokens());

        while(peekToken() && ( 
                (peekToken()->isCharacterCat(Token::CC_OTHER) &&
                        std::isxdigit(peekToken()->value()[0]) &&
                        !std::islower(peekToken()->value()[0])) ||
                (peekToken()->isCharacterCat(Token::CC_LETTER) &&
                        std::isxdigit(peekToken()->value()[0]) &&
                        !std::islower(peekToken()->value()[0]) &&
                        !std::isdigit(peekToken()->value()[0])))) {
            if(result != TEXPP_INT_INV) {
                int v = isdigit(peekToken()->value()[0]) ?
                            peekToken()->value()[0]-'0' :
                            peekToken()->value()[0]-'A'+10;
                if(result<=TEXPP_INT_MAX/16 && result*16<=TEXPP_INT_MAX-v) {
                    result = result*16 + v;
                } else {
                    logger()->log(Logger::ERROR, "Number too big",
                                        *this, lastToken());
                    result = TEXPP_INT_INV;
                }
            }
            nextToken(&node->tokens()); ++digits;
        }

    } else if(peekToken()->isCharacter('\'', Token::CC_OTHER)) {
        nextToken(&node->tokens());
        while(peekToken() && 
                peekToken()->isCharacterCat(Token::CC_OTHER) &&
                        std::isdigit(peekToken()->value()[0]) &&
                peekToken()->value()[0] < '8') {
            if(result != TEXPP_INT_INV) {
                int v = peekToken()->value()[0] - '0';
                if(result<=TEXPP_INT_MAX/8 && result*8<=TEXPP_INT_MAX-v) {
                    result = result*8 + v;
                } else {
                    logger()->log(Logger::ERROR, "Number too big",
                                        *this, lastToken());
                    result = TEXPP_INT_INV;
                }
            }
            nextToken(&node->tokens()); ++digits;
        }

    } else {
        while(peekToken() && 
                peekToken()->isCharacterCat(Token::CC_OTHER) &&
                        std::isdigit(peekToken()->value()[0])) {
            if(result != TEXPP_INT_INV) {
                int v = peekToken()->value()[0] - '0';
                if(result<=TEXPP_INT_MAX/10 && result*10<=TEXPP_INT_MAX-v) {
                    result = result*10 + v;
                } else {
                    logger()->log(Logger::ERROR, "Number too big",
                                        *this, lastToken());
                    result = TEXPP_INT_INV;
                }
            }
            nextToken(&node->tokens()); ++digits;
        }
    }

    if(result == TEXPP_INT_INV)
        result = TEXPP_INT_MAX;

    if(!digits) {
        logger()->log(Logger::ERROR,
            "Missing number, treated as zero",
                                        *this, lastToken());
    }

    node->setValue(result);

    if(helperIsImplicitCharacter(Token::CC_SPACE))
        nextToken(&node->tokens());

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseNormalDimen(bool fil, bool mu)
{
    Node::ptr node(new Node("normal_dimen"));
    if(!peekToken()) {
        logger()->log(Logger::ERROR,
            "Missing number, treated as zero", *this, Token::ptr());
        node->setValue(Dimen(0));
        return node;
    }

    Node::ptr dimen =
        base::Variable::tryParseVariableValue<base::InternalDimen>(*this);
    if(dimen) {
        node->appendChild("internal_dimen", dimen);
        node->setValue(dimen->valueAny());
        resetNoexpand();
        return node;
    }

    // Factor
    Node::ptr factor = parseDimenFactor();
    node->appendChild("factor", factor);
    pair<int, int> val = factor->value(std::make_pair(int(0), int(0)));
    bool overflow = false;

    // <optional_spaces>
    if(helperIsImplicitCharacter(Token::CC_SPACE))
        nextToken(&factor->tokens());

    if(fil) {
        // <fil unit>
        static vector<string> kw_fil(1, "fil");
        static vector<string> kw_l(1, "l");
        Node::ptr fil = parseKeyword(kw_fil);
        if(fil) {
            int level = 1;
            node->appendChild("fil_unit", fil);
            while(true) {
                Node::ptr l = parseKeyword(kw_l);
                if(!l) break;
                std::copy(l->tokens().begin(), l->tokens().end(),
                            std::back_inserter(fil->tokens()));
                if(level<3) {
                    ++level;
                } else {
                    logger()->log(Logger::ERROR,
                        "Illegal unit of measure (replaced by filll)",
                        *this, lastToken());
                }
            }
            fil->setValue("fi" + string(level, 'l'));
            int v = val.first * 0x10000 + val.second;
            if(v >= 0x40000000) {
                logger()->log(Logger::ERROR,
                    "Dimension too large", *this, lastToken());
                v = TEXPP_SCALED_MAX;
            }

            while(helperIsImplicitCharacter(Token::CC_SPACE))
                nextToken(&fil->tokens());

            node->setValue(Dimen(v));
            return node;
        }
    }

    // <internal unit>
    bool i_mu = false;
    bool i_found = false;
    int i_unit = 0;

    Node::ptr iunit =
        base::Variable::tryParseVariableValue<base::InternalInteger>(*this);
    if(iunit) {
        node->appendChild("internal_unit", iunit);
        i_unit = iunit->value(0);
        i_found = true;
    }

    if(!iunit) {
        iunit =
            base::Variable::tryParseVariableValue<base::InternalDimen>(*this);
        if(iunit) {
            node->appendChild("internal_unit", iunit);
            i_unit = iunit->value(Dimen(0)).value;
            i_found = true;
        }
    }

    if(!iunit) {
        iunit =
            base::Variable::tryParseVariableValue<base::InternalGlue>(*this);
        if(iunit) {
            node->appendChild("internal_unit", iunit);
            i_unit = iunit->value(base::Glue(0,0)).width.value;
            i_found = true;
        }
    }

    if(!iunit) {
        iunit =
            base::Variable::tryParseVariableValue<base::InternalMuGlue>(*this);
        if(iunit) {
            node->appendChild("internal_unit", iunit);
            i_unit = iunit->value(base::Glue(1,0)).width.value;
            i_found = true;
            i_mu = true;
        }
    }

    if(!i_found && !mu) {
        static vector<string> kw_internal_units;
        if(kw_internal_units.empty()) {
            kw_internal_units.push_back("em");
            kw_internal_units.push_back("ex");
        }

        iunit = parseKeyword(kw_internal_units);
        if(iunit) {
            node->appendChild("internal_unit", iunit);
            i_unit = 0; // TODO: fontdimen
            i_found = true;
        }
    }

    if(i_found) {
        if(mu != i_mu) {
            logger()->log(Logger::ERROR,
                "Incompatible glue units", *this, lastToken());
        }
        if(i_unit != 0) {
            int v = TEXPP_SCALED_MAX;
            tuple<int,int,bool> p = base::InternalDimen::multiplyIntFrac(
                        i_unit, val.second, 0x10000);
            if(i_unit < 0) { i_unit=-i_unit; val.first=-val.first; }
            if(!p.get<2>() && val.first <= TEXPP_SCALED_MAX/i_unit &&
                        val.first*i_unit <= TEXPP_SCALED_MAX-p.get<0>()) {
                v = val.first*i_unit + p.get<0>();
            } else {
                logger()->log(Logger::ERROR,
                    "Dimension too large", *this, lastToken());
                overflow = true;
                v = TEXPP_SCALED_MAX;
            }
            node->setValue(Dimen(v));
        } else {
            node->setValue(Dimen(0));
        }

        if(iunit && iunit->type() == "keyword")
            if(helperIsImplicitCharacter(Token::CC_SPACE))
                nextToken(&iunit->tokens());

        resetNoexpand();
        return node;
    }

    Node::ptr units;

    if(!mu) {
        // <optional true>
        static vector<string> kw_optional_true;
        if(kw_optional_true.empty()) {
            kw_optional_true.push_back("true");
        }

        Node::ptr optional_true = parseKeyword(kw_optional_true);
        if(optional_true) {
            node->appendChild("optional_true", optional_true);
            int mag = symbol("mag", int(0));
            int activemag = symbol("activemag", mag);
            setSymbol("activemag", activemag);
            if(activemag != mag) {
                logger()->log(Logger::ERROR,
                    "Incompatible magnification (" +
                    boost::lexical_cast<string>(mag) + ");\n" +
                    " the previous value will be retained (" +
                    boost::lexical_cast<string>(activemag) + ")",
                    *this, lastToken());
            }
            if(activemag != 1000) {
                tuple<int, int, bool> p =
                    base::InternalDimen::multiplyIntFrac(
                                val.first, 1000, activemag);
                if(!p.get<2>()) {
                    val.first = p.get<0>();
                    val.second = (1000*val.second +
                                    0x10000*p.get<1>()) / activemag;
                    val.first = val.first + (val.second / 0x10000);
                    val.second = val.second % 0x10000;
                } else {
                    overflow = true;
                }
            }
        }

        // <physical units>
        static int u_scale[][2] = {
            {1,1},          // pt
            {1,1},          // sp
            {7227,100},     // in
            {12,1},         // pc
            {7227,254},     // cm
            {7227,2540},    // mm
            {7227,7200},    // bp
            {1238,1157},    // dd
            {14856,1157},   // cc
        };
        static vector<string> kw_physical_units;
        if(kw_physical_units.empty()) {
            kw_physical_units.push_back("pt");
            kw_physical_units.push_back("sp");
            kw_physical_units.push_back("in");
            kw_physical_units.push_back("pc");
            kw_physical_units.push_back("cm");
            kw_physical_units.push_back("mm");
            kw_physical_units.push_back("bp");
            kw_physical_units.push_back("dd");
            kw_physical_units.push_back("cc");
        }

        units = parseKeyword(kw_physical_units);
        if(units) {
            node->appendChild("physical_unit", units);
            vector<string>::iterator it = std::find(kw_physical_units.begin(),
                        kw_physical_units.end(), units->value(string()));
            assert(it != kw_physical_units.end());
            int n = it - kw_physical_units.begin();
            if(n == 0) { // pt
                // do nothing
                if(val.first > TEXPP_SCALED_MAX/0x10000)
                    overflow = true;
            } else if(n == 1) { // sp
                val.second = val.first % 0x10000;
                val.first  = val.first / 0x10000;
            } else { // neither pt not sp
                tuple<int,int,bool> p = base::InternalDimen::multiplyIntFrac(
                            val.first,u_scale[n][0],u_scale[n][1]);
                overflow = p.get<2>();
                val.first = p.get<0>();
                val.second = (val.second * u_scale[n][0] +
                            p.get<1>() * 0x10000) / u_scale[n][1];
                val.first = val.first + (val.second / 0x10000);
                val.second = val.second % 0x10000;
            }

        } else {
            logger()->log(Logger::ERROR,
                "Illegal unit of measure (pt inserted)", *this, lastToken());
            if(val.first > TEXPP_SCALED_MAX/0x10000)
                overflow = true;
        }

    } else {
        Node::ptr i_node =
            base::Variable::tryParseVariableValue<base::InternalMuGlue>(*this);
        if(i_node) {
            node->appendChild("internal_muunit", i_node);
            int i_unit = i_node->value(base::Glue(1,0)).width.value;

            if(i_unit != 0) {
                int v = TEXPP_SCALED_MAX;
                tuple<int,int,bool> p = base::InternalDimen::multiplyIntFrac(
                            i_unit, val.second, 0x10000);
                if(i_unit < 0) { i_unit=-i_unit; val.first=-val.first; }
                if(!p.get<2>() && val.first <= TEXPP_SCALED_MAX/i_unit &&
                            val.first*i_unit <= TEXPP_SCALED_MAX-p.get<0>()) {
                    v = val.first*i_unit + p.get<0>();
                } else {
                    logger()->log(Logger::ERROR,
                        "Dimension too large", *this, lastToken());
                    overflow = true;
                    v = TEXPP_SCALED_MAX;
                }
                node->setValue(Dimen(v));
            } else {
                node->setValue(Dimen(0));
            }
            resetNoexpand();
            return node;
        }

        // <mu units >
        static vector<string> kw_mu(1, "mu");
        units = parseKeyword(kw_mu);
        if(units) {
            node->appendChild("muunit", units);
        } else {
            logger()->log(Logger::ERROR,
                "Illegal unit of measure (mu inserted)", *this, lastToken());
        }

        if(val.first > TEXPP_SCALED_MAX/0x10000)
            overflow = true;
    }

    int v = val.first * 0x10000 + val.second;
    if(overflow || v >= 0x40000000) {
        logger()->log(Logger::ERROR,
            "Dimension too large", *this, lastToken());
        v = TEXPP_SCALED_MAX;
    }

    node->setValue(Dimen(v));

    if(!units) {
        units = Node::ptr(new Node("unit"));
        node->appendChild("unit", units);
    }
    if(helperIsImplicitCharacter(Token::CC_SPACE))
        nextToken(&units->tokens());

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseDimenFactor()
{
    if(peekToken() && peekToken()->isCharacterCat(Token::CC_OTHER) &&
            (std::isdigit(peekToken()->value()[0]) ||
             peekToken()->value()[0] == '.' ||
             peekToken()->value()[0] == ',')) {

        Node::ptr node(new Node("decimal_constant"));

        int result = 0;
        int frac = 0;
        int digits = 0;

        while(peekToken() && 
                peekToken()->isCharacterCat(Token::CC_OTHER) &&
                        std::isdigit(peekToken()->value()[0])) {
            if(result != TEXPP_INT_INV) {
                int v = peekToken()->value()[0] - '0';
                if(result<=TEXPP_INT_MAX/10 && result*10<=TEXPP_INT_MAX-v) {
                    result = result*10 + v;
                } else {
                    logger()->log(Logger::ERROR, "Number too big",
                                        *this, lastToken());
                    result = TEXPP_INT_INV;
                }
            }
            nextToken(&node->tokens()); ++digits;
        }

        if(result == TEXPP_INT_INV)
            result = TEXPP_INT_MAX;

        if(peekToken() && (peekToken()->isCharacter('.', Token::CC_OTHER) ||
                           peekToken()->isCharacter(',', Token::CC_OTHER))) {
            nextToken(&node->tokens()); ++digits;
            
            string fracDigits;
            while(peekToken() && 
                    peekToken()->isCharacterCat(Token::CC_OTHER) &&
                            std::isdigit(peekToken()->value()[0])) {
                if(fracDigits.size() < 17)
                    fracDigits += peekToken()->value()[0];
                nextToken(&node->tokens());
            }
            string::reverse_iterator rend = fracDigits.rend();
            for(string::reverse_iterator it = fracDigits.rbegin();
                                    it != rend; ++it) {
                frac = (frac + (*it-'0')*0x20000)/10;
            }
            frac = (frac+1)/2;
        }

        if(!digits) {
            logger()->log(Logger::ERROR,
                "Missing number, treated as zero",
                                            *this, lastToken());
        }
        
        node->setValue(std::make_pair(int(result), frac));
        resetNoexpand();
        return node;

    } else {
        Node::ptr node = parseNormalInteger();
        node->setValue(std::make_pair(node->value(0), 0));
        resetNoexpand();
        return node;
    }
}

Node::ptr Parser::parseNumber()
{
    Node::ptr node(new Node("number"));
    node->appendChild("sign", parseOptionalSigns());

    int unsigned_value = 0;
    Node::ptr internal =
        base::Variable::tryParseVariableValue<base::InternalDimen>(*this);
    if(internal) {
        node->appendChild("coerced_dimen", internal);
        unsigned_value = internal->value(Dimen(0)).value;
    }

    if(!internal) {
        internal =
            base::Variable::tryParseVariableValue<base::InternalGlue>(*this);
        if(internal) {
            node->appendChild("coerced_glue", internal);
            unsigned_value = internal->value(base::Glue(0,0)).width.value;
        }
    }

    if(!internal) {
        internal =
            base::Variable::tryParseVariableValue<base::InternalMuGlue>(*this);
        if(internal) {
            node->appendChild("coerced_muglue", internal);
            unsigned_value = internal->value(base::Glue(1,0)).width.value;
            logger()->log(Logger::ERROR,
                "Incompatible glue units", *this, lastToken());
        }
    }

    if(!internal) {
        internal = parseNormalInteger();
        node->appendChild("normal_integer", internal);
        unsigned_value = internal->value(0);
    }

    node->setValue(node->child(0)->value(int(0)) * unsigned_value);

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseDimen(bool fil, bool mu)
{
    Node::ptr node(new Node(mu ? "mudimen" : "dimen"));
    node->appendChild("sign", parseOptionalSigns());

    bool intern = false;
    bool intern_mu = false;

    int unsigned_value = 0;
    Node::ptr internal =
        base::Variable::tryParseVariableValue<base::InternalGlue>(*this);
    if(internal) {
        node->appendChild("coerced_glue", internal);
        unsigned_value = internal->value(base::Glue(0,0)).width.value;
        intern = true;
    }

    if(!internal) {
        internal =
            base::Variable::tryParseVariableValue<base::InternalMuGlue>(*this);
        if(internal) {
            node->appendChild("coerced_muglue", internal);
            unsigned_value = internal->value(base::Glue(1,0)).width.value;
            intern = true;
            intern_mu = true;
        }
    }

    if(intern) {
        if(intern_mu != mu) {
            logger()->log(Logger::ERROR,
                "Incompatible glue units", *this, lastToken());
        }
    } else {
        internal = parseNormalDimen(fil, mu);
        node->appendChild(mu ? "normal_mudimen" : "normal_dimen", internal);
        unsigned_value = internal->value(Dimen(0)).value;
    }

    node->setValue(
        Dimen(node->child(0)->value(0) * unsigned_value));

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseGlue(bool mu)
{
    Node::ptr node(new Node(mu ? "muglue" : "glue"));
    node->appendChild("sign", parseOptionalSigns());
    int sign = node->child(0)->value(int(0));

    base::Glue glue(mu,0);
    bool intern = false;
    bool intern_mu = false;

    Node::ptr internal =
        base::Variable::tryParseVariableValue<base::InternalGlue>(*this);
    if(internal) {
        node->appendChild("internal_glue", internal);
        glue = internal->value(base::Glue(0,0));
        intern = true;
    }

    if(!internal) {
        internal =
            base::Variable::tryParseVariableValue<base::InternalMuGlue>(*this);
        if(internal) {
            node->appendChild("internal_glue", internal);
            glue = internal->value(base::Glue(1,0));
            intern = true;
            intern_mu = true;
        }
    }

    if(intern) {
        if(intern_mu != mu) {
            logger()->log(Logger::ERROR,
                "Incompatible glue units", *this, lastToken());
            glue.mu = mu;
        }

        glue.width.value *= sign;
        glue.stretch.value *= sign;
        glue.shrink.value *= sign;

        node->setValue(glue);

        resetNoexpand();
        return node;
    }

    Node::ptr width = parseDimen(false, mu);
    node->appendChild("width", width);
    glue.width = Dimen(sign * width->value(Dimen(0)).value);

    Node::ptr dimenStretch;
    static vector<string> kw_plus(1, "plus");
    Node::ptr stretch = parseOptionalKeyword(kw_plus);
    node->appendChild("stretch", stretch);
    if(stretch->value(string()) == "plus") {
        dimenStretch = parseDimen(true, mu);
        node->appendChild("stretch_dimen", dimenStretch);
        stretch->setValue(dimenStretch->valueAny());
        stretch->setType("stretch");

        glue.stretch = stretch->value(Dimen(0));

        Node::ptr fil = dimenStretch->child(!mu ? "normal_dimen":
                                                  "normal_mudimen");
        if(fil) fil = fil->child("fil_unit");
        if(fil) {
            string v = fil->value(string());
            glue.stretchOrder = std::count(v.begin(), v.end(), 'l');
        }
    }

    Node::ptr dimenShrink;
    static vector<string> kw_minus(1, "minus");
    Node::ptr shrink = parseOptionalKeyword(kw_minus);
    node->appendChild("shrink", shrink);
    if(shrink->value(string()) == "minus") {
        dimenShrink = parseDimen(true, mu);
        node->appendChild("shrink_dimen", dimenShrink);

        shrink->setValue(dimenShrink->valueAny());
        shrink->setType("shrink");

        glue.shrink = shrink->value(Dimen(0));

        Node::ptr fil = dimenShrink->child(!mu ? "normal_dimen":
                                                 "normal_mudimen");
        if(fil) fil = fil->child("fil_unit");
        if(fil) {
            string v = fil->value(string());
            glue.shrinkOrder = std::count(v.begin(), v.end(), 'l');
        }
    }

    node->setValue(glue);

    resetNoexpand();
    return node;
}


Node::ptr Parser::parseBalancedText(bool expand,
                    int paramCount, Token::ptr nameToken)
{
    Node::ptr node(new Node("balanced_text"));
    Token::list_ptr tokens(new Token::list);

    int level = 0;
    Token::ptr token;
    while(token = peekToken(expand)) {
        if(token->isCharacterCat(Token::CC_BGROUP)) {
            ++level;
        } else if(token->isCharacterCat(Token::CC_EGROUP)) {
            if(--level < 0) break;
        }

        tokens->push_back(nextToken(&node->tokens(), expand));

        if(paramCount >= 0 && token->isCharacterCat(Token::CC_PARAM)) {
            Token::ptr nToken = peekToken(expand);
            if(!nToken || !nToken->isCharacterCat(Token::CC_PARAM)) {
                char ch = nToken && nToken->isCharacter() ?
                            nToken->value()[0] : 0;
                int n = std::isdigit(ch) ? ch - '0' : 0;
                if(n <= 0 || n > paramCount) {
                    logger()->log(Logger::ERROR,
                        "Illegal parameter number in definition of "
                        + (nameToken ? nameToken->texRepr(this) :
                           escapestr() + "undefined"),
                        *this, lastToken());
                    tokens->push_back(Token::create(
                        token->type(), token->catCode(), token->value()));
                }
            } else if(nToken) {
                tokens->push_back(nextToken(&node->tokens(), expand));
            }
        }
    }
    node->setValue(tokens);
    return node;
}

Node::ptr Parser::parseFiller(bool expand)
{
    Node::ptr filler(new Node("filler"));
    while(peekToken(expand)) {
        if(helperIsImplicitCharacter(Token::CC_SPACE, expand)) {
            nextToken(&filler->tokens(), expand);
            continue;
        } else if(peekToken(expand)->isControl()) {
            //Command::ptr obj = symbol(peekToken(expand), Command::ptr());
            Command::ptr cmd = symbolCommand<base::Relax>(peekToken(expand));
            if(cmd) {
                nextToken(&filler->tokens(), expand);
                continue;
            }
        }
        break;
    }

    resetNoexpand();
    return filler;
}

Node::ptr Parser::parseGeneralText(bool expand, bool implicitLbrace)
{
    Node::ptr node(new Node("general_text"));

    // parse filler (always expanded)
    node->appendChild("filler", parseFiller(true));

    // parse left_brace
    Node::ptr left_brace(new Node("left_brace"));
    node->appendChild("left_brace", left_brace);
    if(peekToken(expand) && (
       (implicitLbrace &&
            helperIsImplicitCharacter(Token::CC_BGROUP, expand)) ||
       (!implicitLbrace &&
            peekToken(expand)->isCharacterCat(Token::CC_BGROUP)))) {
        left_brace->setValue(nextToken(&left_brace->tokens(), expand));
    } else {
        logger()->log(Logger::ERROR, "Missing { inserted",
                        *this, lastToken());
        left_brace->setValue(Token::create(
                    Token::TOK_CHARACTER, Token::CC_BGROUP, "{"));
    }

    node->appendChild("balanced_text", parseBalancedText(expand));

    // parse right_brace
    Node::ptr right_brace(new Node("right_brace"));
    node->appendChild("right_brace", right_brace);
    if(peekToken(expand) &&
            peekToken(expand)->isCharacterCat(Token::CC_EGROUP)) {
        right_brace->setValue(nextToken(&right_brace->tokens(), expand));
    } else {
        // TODO: error
        right_brace->setValue(Token::create(
                    Token::TOK_CHARACTER, Token::CC_EGROUP, "}"));
    }

    resetNoexpand();
    return node;
}

Node::ptr Parser::parseFileName()
{
    static bool parsing = false;

    Node::ptr node(new Node("file_name"));

    if(parsing)
        return node;
    else
        parsing = true;

    string fileName;

    while(helperIsImplicitCharacter(Token::CC_SPACE))
        nextToken(&node->tokens());

    while(peekToken() && peekToken()->isCharacter() &&
            !peekToken()->isCharacterCat(Token::CC_SPACE)) {
        Token::ptr letter = nextToken(&node->tokens());
        fileName += letter->value();
    }

    if(helperIsImplicitCharacter(Token::CC_SPACE))
        nextToken(&node->tokens());

    node->setValue(fileName);

    resetNoexpand();

    parsing = false;
    return node;
}

Node::ptr Parser::parseTextWord()
{
    string value;
    Node::ptr node(new Node("text_word"));
    while(peekToken() && peekToken()->isCharacterCat(Token::CC_LETTER)) {
        if(mode() != MATH && mode() != DMATH)
            processTextCharacter(peekToken()->value()[0], peekToken());
        if(!value.empty()) traceCommand(peekToken());

        value += peekToken()->value();
        nextToken(&node->tokens());
    }
    node->setValue(value);

    return node;
}

void Parser::traceCommand(Token::ptr token, bool expanding)
{
    int tracingcommands = symbol("tracingcommands", int(0));
    if(tracingcommands > 0) {
        string str;
        if(token->isControl()) {
            Command::ptr cmd = symbol(token, Command::ptr());
            if(dynamic_pointer_cast<base::TheMacro>(cmd)) {
                Command::ptr c = currentCommand();
                if(mode() == NULLMODE ||
                        dynamic_pointer_cast<base::Write>(c) ||
                        dynamic_pointer_cast<base::Message>(c) ||
                        (dynamic_pointer_cast<base::Def>(c) &&
                         static_pointer_cast<base::Def>(c)->expand())) {
                    return;
                }
            }
            if(dynamic_pointer_cast<Macro>(cmd)) {
                if(expanding) {
                    if(tracingcommands < 2) return;
                    if(dynamic_pointer_cast<base::UserMacro>(cmd)) return;
                    str += cmd->texRepr(this);
                } else {
                    str += escapestr();
                    str += "relax";
                }
                //std::remove(str.begin(), str.end(), '\n');
            } else if(cmd) {
                str += cmd->texRepr(this);
            } else {
                str = "undefined";
            }
        } else {
            str = token->meaning(this);
        }
        if(m_prevMode != m_mode) {
            str = modeName() + " mode: " + str;
            m_prevMode = m_mode;
        }
        //logger()->log(Logger::TRACING, str, *this, lastToken());
        logger()->log(Logger::TRACING, str, *this, token);
    }
}

Node::ptr Parser::parseGroup(GroupType groupType)
{
    GroupType prevGroupType = m_currentGroupType;
    m_currentGroupType = groupType;

    Node::ptr node(new Node("group"));

    if(groupType == GROUP_NORMAL) {
        if(helperIsImplicitCharacter(Token::CC_BGROUP)) {
            node->appendChild("group_begin", parseToken());
        } else {
            logger()->log(Logger::ERROR, "Missing { inserted",
                                    *this, lastToken());
            Node::ptr left_brace(new Node("token"));
            left_brace->setValue(Token::create(
                        Token::TOK_CHARACTER, Token::CC_BGROUP, "{"));
            node->appendChild("group_begin", left_brace);
        }
        if(m_afterassignmentToken &&
                dynamic_pointer_cast<base::Setbox>(currentCommand())) {
            // We are in \setbox=\?box{, afterassignment token should
            // be inserted here
            Token::list tokens(1, m_afterassignmentToken);
            pushBack(&tokens);
            m_afterassignmentToken.reset();
        }
    } else if(groupType == GROUP_SUPER) {
        assert(symbolCommand<Begingroup>(peekToken()));
        node->appendChild("group_begin", parseToken());
    } else if(groupType == GROUP_MATH) {
        assert(helperIsImplicitCharacter(Token::CC_MATHSHIFT));
        node->appendChild("group_begin", parseToken());
    } else if(groupType == GROUP_DMATH) {
        assert(helperIsImplicitCharacter(Token::CC_MATHSHIFT));
        node->appendChild("group_begin", parseDMathToken());
    }

    while(true) {
        if(!peekToken()) {
            if(groupType == GROUP_MATH || groupType == GROUP_DMATH) {
                Node::ptr group_end(new Node("group_end"));
                Token::ptr t(Token::create(Token::TOK_CHARACTER,
                            Token::CC_MATHSHIFT, "$"));
                group_end->setValue(t);
                traceCommand(t);

                node->appendChild("group_end", group_end);
                logger()->log(Logger::ERROR,
                        "Missing $ inserted", *this, lastToken());

                if(groupType == GROUP_DMATH)
                    logger()->log(Logger::ERROR,
                        "Display math should end with $$",
                        *this, lastToken());
            }
            break;
        }

        traceCommand(peekToken());

        if(helperIsImplicitCharacter(Token::CC_EGROUP)) {
            if(groupType == GROUP_NORMAL) {
                node->appendChild("group_end", parseToken());
                break;
            } else {
                string msg;
                switch(groupType) {
                    case GROUP_DOCUMENT:
                        msg = "Too many }'s";
                        break;
                    case GROUP_MATH:
                    case GROUP_DMATH:
                        msg = "Extra }, or forgotten $";
                        break;
                    case GROUP_SUPER:
                        msg = "Extra }, or forgotten " +
                            Token(Token::TOK_CONTROL, Token::CC_ESCAPE,
                                    "\\endgroup").texRepr(this);
                        break;
                    default:
                        msg = "Extra }";
                }
                logger()->log(Logger::ERROR, msg, *this, lastToken());
                node->appendChild("ignored_egroup", parseToken());
            }

        } else if(helperIsImplicitCharacter(Token::CC_BGROUP)) {
            beginGroup();
            node->appendChild("group", parseGroup(GROUP_NORMAL));
            //pushBack(&m_aftergroupTokens);
            //m_aftergroupTokens.clear();
            endGroup();

        } else if(helperIsImplicitCharacter(Token::CC_MATHSHIFT)) {

            if(groupType == GROUP_MATH) {
                node->appendChild("group_end", parseToken());
                break;
            } else if(groupType == GROUP_DMATH) {
                Node::ptr dmathNode = parseDMathToken();
                node->appendChild("group_end", dmathNode);
                if(helperIsImplicitCharacter(Token::CC_SPACE, false)) {
                    nextToken(&dmathNode->tokens());
                }
                break;
            }

            Node::ptr t1 = parseToken();

            // XXX: is the following line correct ?
            bool dmath = helperIsImplicitCharacter(Token::CC_MATHSHIFT,
                                                                false);
            pushBack(&t1->tokens());

            if(mode() != HORIZONTAL) {
                setMode(HORIZONTAL);
                traceCommand(t1->value(Token::ptr()));
            }

            beginGroup();
            Mode prevMode = mode();
            setMode(dmath ? DMATH : MATH);

            setSymbol("fam", int(-1));

            if(dmath) {
                setSymbol("predisplaysize", Dimen(0));
                setSymbol("displaywidth", Dimen(0));
                setSymbol("displayindent", Dimen(0));
            }

            node->appendChild("inline_math", parseGroup(
                    dmath ? GROUP_DMATH : GROUP_MATH));

            setMode(prevMode);

            endGroup();

        } else if(peekToken()->isCharacterCat(Token::CC_LETTER)) {
            node->appendChild("text_word", parseTextWord());

        } else if(peekToken()->isCharacterCat(Token::CC_SPACE)) {
            if(mode() == HORIZONTAL || mode() == RHORIZONTAL) {
                node->appendChild("text_space", parseTextCharacter());
            } else {
                node->appendChild("space", parseToken());
            }

        } else if(peekToken()->isCharacterCat(Token::CC_OTHER)) {
            node->appendChild("text_character", parseTextCharacter());

        } else if(peekToken()->isCharacterCat(Token::CC_PARAM)) {
            m_logger->log(Logger::ERROR,
                "You can't use `" + peekToken()->meaning(this) + "' in " +
                modeName() + " mode", *this, lastToken());
            node->appendChild("error_param", parseToken());

        } else if(peekToken()->isControl()) {
            Command::ptr cmd = symbol(peekToken(), Command::ptr());
            Node::ptr cmdNode;
            if(cmd) {
                if(dynamic_pointer_cast<Begingroup>(cmd)) {
                    beginGroup();
                    node->appendChild("group", parseGroup(GROUP_SUPER));
                    //pushBack(&m_aftergroupTokens);
                    //m_aftergroupTokens.clear();
                    endGroup();
                } else if(dynamic_pointer_cast<Endgroup>(cmd)) {
                    if(groupType == GROUP_SUPER) {
                        node->appendChild("group_end", parseToken());
                        break;
                    } else {
                        string msg;
                        Token::ptr t;
                        if(groupType == GROUP_NORMAL) {
                            msg = "Missing } inserted";
                            t = Token::create(Token::TOK_CHARACTER,
                                        Token::CC_EGROUP, "}");
                        } else if(groupType == GROUP_MATH) {
                            msg = "Missing $ inserted";
                            t = Token::create(Token::TOK_CHARACTER,
                                        Token::CC_MATHSHIFT, "$");
                        } else if(groupType == GROUP_DMATH) {
                            logger()->log(Logger::ERROR, 
                                   "Missing $ inserted", *this, lastToken());
                            msg = "Display math should end with $$";
                            t = Token::create(Token::TOK_CHARACTER,
                                        Token::CC_MATHSHIFT, "$");
                        } else {
                            msg = "Extra " + Token(Token::TOK_CONTROL,
                                Token::CC_ESCAPE, "\\endgroup").texRepr(this);
                        }
                        logger()->log(Logger::ERROR, msg, *this, lastToken());
                        if(groupType != GROUP_DOCUMENT) {
                            Node::ptr group_end(new Node("group_end"));
                            if(t) {
                                group_end->setValue(t);
                                traceCommand(t);
                            }

                            node->appendChild("group_end", group_end);
                            break;
                        } else {
                            node->appendChild("extra_endgroup", parseToken());
                        }
                    }
                } else {
                    Mode prevMode = mode();
                    cmd->presetMode(*this);
                    if(mode() != prevMode)
                        traceCommand(peekToken());

                    cmdNode = parseCommand(cmd);

                    if(m_customGroupBegin) {
                        m_customGroupBegin = false;
                        string type = m_customGroupType;
                        Node::ptr customGroup = parseGroup(GROUP_CUSTOM);
                        customGroup->setType(type);
                        customGroup->children().insert(
                                customGroup->children().begin(),
                                std::make_pair("control", cmdNode));
                        node->appendChild("custom_group", customGroup);
                    } else if(m_customGroupEnd) {
                        m_customGroupEnd = false;
                        if(groupType == GROUP_CUSTOM) {
                            node->appendChild("control", cmdNode);
                            break;
                        } else {
                            logger()->log(Logger::ERROR,
                                "Extra " + cmd->texRepr(this), *this, lastToken());
                        }
                    } else {
                        node->appendChild("control", cmdNode);
                    }
                }
            } else {
                /*m_logger->log(Logger::ERROR, "Undefined control sequence",
                                                *this, lastToken());*/
                cmdNode = parseToken();
                node->appendChild("unexpanded_macro", cmdNode);
                //node->appendChild("error_unknown_control",
                //                                parseToken());
            }
        } else {
            node->appendChild("other_token", parseToken());
        }
    }

    m_currentGroupType = prevGroupType;
    return node;
}

Node::ptr Parser::parse()
{
    if(!lexer()->fileName().empty()) {
        string fname = lexer()->fileName();
        logger()->log(Logger::MESSAGE,
            "**" + fname + "\n", *this, Token::ptr());
        bool p = std::find(fname.begin(), fname.end(), PATH_SEP)!=fname.end();
        logger()->log(Logger::MESSAGE,
            (p ? "(" : "(." + string(1, PATH_SEP))
            + lexer()->fileName() + "\n", *this, Token::ptr());
    }

    setMode(VERTICAL);
    Node::ptr document = parseGroup(GROUP_DOCUMENT);
    document->setType("document");
    
    // Some skipped tokens may still exists even when
    // peekToken reports EOF. Lets add that tokens to the last node.
    Node::ptr node = document;
    while(node->childrenCount() > 0)
        node = node->child(node->childrenCount()-1);

    nextToken(&node->tokens());

    if(!lexer()->fileName().empty()) {
        logger()->log(Logger::MESSAGE,
            " )", *this, Token::ptr());
    }

    return document;
}

} // namespace texpp

