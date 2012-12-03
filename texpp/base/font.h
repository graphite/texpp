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

#ifndef __TEXPP_BASE_FONT_H
#define __TEXPP_BASE_FONT_H

#include <texpp/common.h>
#include <texpp/command.h>

#include <texpp/base/variable.h>
#include <texpp/base/integer.h>
#include <texpp/base/dimen.h>

namespace texpp {
namespace base {

struct FontInfo
{
    typedef shared_ptr<FontInfo> ptr;

    string selector;
    string file;
    Dimen at;

    FontInfo() {}
    explicit FontInfo(string s, string f = "nullfont", Dimen a = Dimen(0))
        : selector(s), file(f), at(a) {}

};

extern shared_ptr<FontInfo> defaultFontInfo;

class FontVariable: public Variable
{
public:
    FontVariable(const string& name,
        const any& initValue = any(defaultFontInfo))
        : Variable(name, initValue) {}

    static string fontToString(const FontInfo& fontInfo);
};

class FontSelector: public FontVariable
{
public:
    FontSelector(const string& name,
        const any& initValue = any(defaultFontInfo))
        : FontVariable(name, initValue) {}

    FontInfo::ptr initFontInfo() const {
        return m_initValue.type() == typeid(FontInfo::ptr) ?
            *unsafe_any_cast<FontInfo::ptr>(&m_initValue) : defaultFontInfo;
    }

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);

    string texRepr(Parser* parser = NULL) const;
};

class Font: public FontVariable
{
public:
    Font(const string& name,
        const any& initValue = any(defaultFontInfo))
        : FontVariable(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
};

class FontFamily: public FontVariable
{
public:
    FontFamily(const string& name,
        const any& initValue = any(defaultFontInfo))
        : FontVariable(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
    string parseName(Parser& parser, shared_ptr<Node> node);
};

class FontChar: public SpecialInteger
{
public:
    FontChar(const string& name, const any& initValue = any(0))
        : SpecialInteger(name, initValue) {}

    string parseName(Parser& parser, shared_ptr<Node> node);
};

class FontDimen: public SpecialDimen
{
public:
    FontDimen(const string& name, const any& initValue = any(0))
        : SpecialDimen(name, initValue) {}

    bool invokeOperation(Parser& parser,
                shared_ptr<Node> node, Operation op, bool global);
    string parseName(Parser& parser, shared_ptr<Node> node);
};

class FontnameMacro: public Macro
{
public:
    explicit FontnameMacro(const string& name): Macro(name) {}
    bool expand(Parser& parser, shared_ptr<Node> node);
};

} // namespace base
} // namespace texpp

#endif

