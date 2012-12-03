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

#include <texpp/base/font.h>
#include <texpp/parser.h>

namespace texpp {
namespace base {

shared_ptr<FontInfo> defaultFontInfo(new FontInfo("\\nullfont", "nullfont"));

string FontSelector::texRepr(Parser*) const
{
    return "select font " + fontToString(*initFontInfo());
}

string FontVariable::fontToString(const FontInfo& fontInfo)
{
    string str(fontInfo.file);
    if(fontInfo.at.value) {
        str += " at " + InternalDimen::dimenToString(fontInfo.at);
    }
    return str;
}

bool FontSelector::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == EXPAND) {
        string str = initFontInfo()->selector;
        string escape = parser.escapestr();
        if(!str.empty() && str[0] == '\\') {
            str = escape + str.substr(1);
        } else {
            str = escape + "FONT" + str;
        }
        node->setValue(str + ' ');
        return true;
    } else if(op == ASSIGN) {
        parser.setSymbol("font", initFontInfo(), global);
        return true;
    } else if(op == GET) {
        node->setValue(initFontInfo());
        return true;
    }
    return false;
}

bool Font::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == EXPAND) {
        string name = parseName(parser, node);
        FontInfo::ptr fontInfo = parser.symbol(name, defaultFontInfo);

        string str = fontInfo->selector;
        string escape = parser.escapestr();
        if(!str.empty() && str[0] == '\\') {
            str = escape + str.substr(1);
        } else {
            str = escape + "FONT" + str;
        }
        node->setValue(str + ' ');
        return true;

    } else if(op == ASSIGN) {
        string name = parseName(parser, node);

        Node::ptr lvalue = parser.parseControlSequence(false);
        Token::ptr ltoken = lvalue->value(Token::ptr());
        parser.lockToken(ltoken);

        node->appendChild("lvalue", lvalue);
        node->appendChild("equals", parser.parseOptionalEquals());

        Node::ptr fileName = parser.parseFileName();
        node->appendChild("file_name", fileName);

        Dimen at = Dimen(0);
        static vector<string> kw_at;
        if(kw_at.empty()) {
            kw_at.push_back("at");
            kw_at.push_back("scaled");
        }

        Node::ptr atKw = parser.parseOptionalKeyword(kw_at);
        node->appendChild("at_clause", atKw);

        if(atKw->value(string()) == "at") {
            Node::ptr atNode = parser.parseDimen();
            node->appendChild("at", atNode);

            at = atNode->value(Dimen(0));
            if(at.value <= 0 || at.value >= 0x8000000) {
                parser.logger()->log(Logger::ERROR,
                    "Improper `at' size (" +
                    InternalDimen::dimenToString(at) +
                    + "), replaced by 10pt",
                    parser, parser.lastToken());
                at.value = 655360;
            }

        } else if(atKw->value(string()) == "scaled") {
            Node::ptr scaledNode = parser.parseNumber();
            node->appendChild("scaled", scaledNode);

            int scaled = scaledNode->value(int(0));
            if(scaled <= 0 || scaled > 32768) {
                parser.logger()->log(Logger::ERROR,
                    "Illegal magnification has been changed to 1000 (" +
                    boost::lexical_cast<string>(scaled) + ")",
                    parser, parser.lastToken());
                scaled = 1000;
            }

            // TODO: take actual font size into account !
            at.value =
                InternalDimen::multiplyIntFrac(655360, scaled, 1000).get<0>();
        }

        FontInfo::ptr fontInfo(new FontInfo(ltoken->value(),
                                fileName->value(string()), at));
        
        node->setValue(fontInfo);
        parser.setSymbol(ltoken->value(),
            Command::ptr(new FontSelector(ltoken->value(), fontInfo)),
            global);

        parser.lockToken(Token::ptr());
        return true;

    } else if(op == GET) {
        string name = parseName(parser, node);
        const any& ret = parser.symbolAny(name);
        node->setValue(ret.empty() ? m_initValue : ret);
        return true;
    }
    return false;
}

bool FontFamily::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == EXPAND) {
        string name = parseName(parser, node);
        FontInfo::ptr fontInfo = parser.symbol(name, defaultFontInfo);

        string str = fontInfo->selector;
        string escape = parser.escapestr();
        if(!str.empty() && str[0] == '\\') {
            str = escape + str.substr(1);
        } else {
            str = escape + "FONT" + str;
        }
        node->setValue(str + ' ');
        return true;

    } else if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());

        Node::ptr rvalue =
            Variable::tryParseVariableValue<base::FontVariable>(parser);
        if(!rvalue) {
            parser.logger()->log(Logger::ERROR,
                "Missing font identifier",
                parser, parser.lastToken());
            rvalue = Node::ptr(new Node("error_missing_font"));
            rvalue->setValue(defaultFontInfo);
        }

        node->appendChild("rvalue", rvalue);
        node->setValue(rvalue->valueAny());
        parser.setSymbol(name, rvalue->valueAny(), global);

        return true;

    } else if(op == GET) {
        string name = parseName(parser, node);
        const any& ret = parser.symbolAny(name);
        node->setValue(ret.empty() ? m_initValue : ret);
        return true;
    }
    return false;
}

string FontFamily::parseName(Parser& parser, shared_ptr<Node> node)
{
    shared_ptr<Node> number = parser.parseNumber();
    node->appendChild("family_number", number);
    int n = number->value(int(0));

    if(n < 0 || n > 15) {
        parser.logger()->log(Logger::ERROR,
            "Bad number (" + boost::lexical_cast<string>(n) + ")",
            parser, parser.lastToken());
        n = 0;
    }

    string s = this->name().substr(1) + boost::lexical_cast<string>(n);
    parser.setSymbolDefault(s, m_initValue);
    return s;
}

string FontChar::parseName(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr font =
        Variable::tryParseVariableValue<base::FontVariable>(parser);
    if(!font) {
        parser.logger()->log(Logger::ERROR,
            "Missing font identifier",
            parser, parser.lastToken());
        font = Node::ptr(new Node("error_missing_font"));
        font->setValue(defaultFontInfo);
    }
    node->appendChild("variable_font", font);

    string s = name().substr(1) + font->value(defaultFontInfo)->selector;
    parser.setSymbolDefault(s, m_initValue);
    return s;
}

string FontDimen::parseName(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr number = parser.parseNumber();
    node->appendChild("variable_number", number);
    int n = number->value(int(0));

    Node::ptr font =
        Variable::tryParseVariableValue<base::FontVariable>(parser);
    if(!font) {
        parser.logger()->log(Logger::ERROR,
            "Missing font identifier",
            parser, parser.lastToken());
        font = Node::ptr(new Node("error_missing_font"));
        font->setValue(defaultFontInfo);
    }
    node->appendChild("variable_font", font);
    FontInfo::ptr fontInfo = font->value(defaultFontInfo);

    if(n <= 0/* || n > 7*/) { // TODO: read the number of dimen params
        // TODO: the following calc should be one function
        string str = fontInfo->selector;
        string escape = parser.escapestr();
        if(!str.empty() && str[0] == '\\') {
            str = escape + str.substr(1);
        } else {
            str = escape + "FONT" + str;
        }
        parser.logger()->log(Logger::ERROR,
            "Font " + str + " has only 7 fontdimen parameters", // TODO: 7?
            parser, parser.lastToken());
        n = 0;
    }

    string s = name().substr(1) + boost::lexical_cast<string>(n)
                            + fontInfo->selector;
    parser.setSymbolDefault(s, m_initValue);
    return s;
}

bool FontDimen::invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global)
{
    if(op == ASSIGN) {
        string name = parseName(parser, node);

        node->appendChild("equals", parser.parseOptionalEquals());
        Node::ptr rvalue = parser.parseDimen();
        node->appendChild("rvalue", rvalue);

        node->setValue(rvalue->valueAny());
        if(name.substr(0, 11) != "fontdimen0\\")
            parser.setSymbol(name, rvalue->valueAny(), true); // global
        return true;
    } else {
        return SpecialDimen::invokeOperation(parser, node, op, global);
    }
}

bool FontnameMacro::expand(Parser& parser, shared_ptr<Node> node)
{
    Node::ptr font =
        Variable::tryParseVariableValue<base::FontVariable>(parser);
    if(!font) {
        parser.logger()->log(Logger::ERROR,
            "Missing font identifier",
            parser, parser.lastToken());
        font = Node::ptr(new Node("error_missing_font"));
        font->setValue(defaultFontInfo);
    }
    node->appendChild("font", font);

    node->setValue(stringToTokens(
        FontVariable::fontToString(*font->value(defaultFontInfo))));

    return true;
}

} // namespace base
} // namespace texpp


