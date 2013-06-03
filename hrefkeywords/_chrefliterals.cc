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
#include <boost/python/stl_iterator.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "english_stem.h"
#include "french_stem.h"
#include "german_stem.h"
#include "finnish_stem.h"
#include "swedish_stem.h"
#include "spanish_stem.h"
#include "dutch_stem.h"
#include "danish_stem.h"
#include "italian_stem.h"
#include "norwegian_stem.h"
#include "portuguese_stem.h"
#include "russian_stem.h"

#include <boost/tuple/tuple.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <set>
#include <string>
#include <fstream>
#include <sstream>

#include <cstring>
#include <cctype>

#include <iostream>

#include <texpp/parser.h>
#include <unicode/stringpiece.h>
#include <unicode/unistr.h>
#include <unicode/normalizer2.h>
#include <unicode/schriter.h>

#include <locale.h>
#include <wchar.h>

using namespace boost::python;
using namespace texpp;

class WordsDict {
public:
    WordsDict(string filename, size_t abbrMaxLen)
        : _abbrMaxLen(abbrMaxLen)
    {
        std::ifstream wordsfile(filename.c_str());
        string word;
        while(wordsfile.good()) {
            std::getline(wordsfile, word);
            size_t s = word.size();
            if(s > 1 && s <= abbrMaxLen &&
                    (s < 2 || word.substr(s-2) != "'s") ) {
                _words.insert(word);
            }
        }
    }

    size_t abbrMaxLen() const { return _abbrMaxLen; }

    void insert(string word) { _words.insert(word); }
    bool contains(string word) const { return _words.count(word); }

protected:
    std::set<string> _words;
    size_t _abbrMaxLen;
};

inline bool _isglue(wchar_t ch) {
    return iswdigit(ch) || ch == L'-' || ch == L'/';
}

inline bool _isIgnored(wchar_t ch) {
    return ch == L'~' || ch == L'-' || ch == L'/' || iswspace(ch);
}

inline bool _isIgnoredWord(const string& word) {
    return word == "the" || word == "a" || word == "an" ||
           word == "The" || word == "A" || word == "An";
}

string wStrToStr(std::wstring wstr, bool toLower=false)
{
    string result("");
    char buf[MB_CUR_MAX + 1];
    std::mbstate_t state = std::mbstate_t();
    setlocale(LC_ALL, "en_US.UTF-8");
    for (size_t i=0; i < wstr.length(); i++) {
        size_t len = wcrtomb(buf, toLower? towlower(wstr[i]) : wstr[i], &state);
        if (len <= 0 || len > MB_CUR_MAX)
            break;
        buf[len] = 0;
        result += buf;
    }
    return result;
}

std::wstring strToWStr(string input)
{
    setlocale(LC_ALL, "en_US.UTF-8");
    const char* position = input.c_str();
    std::mbstate_t state = std::mbstate_t();
    wchar_t wChar;
    std::wstring result;
    while (*position) {
        size_t len = mbrtowc(&wChar, position, MB_CUR_MAX, &state);
        if (len <= 0 || len > MB_CUR_MAX)
            break;
        position += len;
        result += wChar;
    }
    return result;
}

std::wstring stem_wstring(std::wstring input, bool multilang=false)
{
    std::wstring input_backup(input);
    stemming::english_stem<> StemEnglish;
    StemEnglish(input);
    if (input != input_backup && input.length() != 0)
        return input;
    input = input_backup;
    if (multilang)
    {
        stemming::french_stem<> StemFrench;
        StemFrench(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
        stemming::german_stem<> StemGerman;
        StemGerman(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
        stemming::spanish_stem<> StemSpanish;
        StemSpanish(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
        stemming::italian_stem<> StemItalian;
        StemItalian(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
        stemming::portuguese_stem<> StemPortuguese;
        StemPortuguese(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
        stemming::dutch_stem<> StemDutch;
        StemDutch(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
        stemming::danish_stem<> StemDanish;
        StemDanish(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
        stemming::finnish_stem<> StemFinnish;
        StemFinnish(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
        stemming::norwegian_stem<> StemNorwegian;
        StemNorwegian(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
        stemming::swedish_stem<> StemSwedish;
        StemSwedish(input);
        if (input != input_backup && input.length() != 0)
            return input;
        input = input_backup;
    }
    stemming::russian_stem<> StemRussian;
    StemRussian(input);
    if (input != input_backup && input.length() != 0)
        return input;
    return input_backup;
}

string normLiteral(string literal,
        const WordsDict* wordsDict, const dict& preWhiteList,
        const dict& postWhiteList, bool multilang)
{
    stemming::english_stem<> StemEnglish;
    std::wstring nWLiteral;
    size_t wordStart = string::npos;
    size_t lastDot = string::npos;
    string lowercaseLiteral(""), unicodeNormLiteral;;
    setlocale(LC_ALL, "en_US.UTF-8");
    /* Decompose unicode chars to the basic ones */
    UErrorCode ecode = U_ZERO_ERROR;
    const Normalizer2* normalizer = icu::Normalizer2::getInstance(NULL, "nfkc", UNORM2_DECOMPOSE, ecode);
    UnicodeString final;
    UnicodeString ULiteral = icu::UnicodeString::fromUTF8(StringPiece(literal));
    UnicodeString normULiteral = normalizer->normalize(ULiteral, ecode);
    StringCharacterIterator iter(normULiteral);
    while(iter.hasNext())
    {
        if (u_isbase(iter.current()) || u_isblank(iter.current()) || u_ispunct(iter.current()))
            final += iter.current();
        iter.next();
    }
    final.toUTF8String(unicodeNormLiteral);
    std::wstring wLiteral = strToWStr(unicodeNormLiteral);
    size_t s = wLiteral.length();
    /* convert to lowercase and check the whitelist */
    if(preWhiteList.has_key(wStrToStr(wLiteral, true))) return literal;

    /* TODO: handle 's */
    for(size_t n=0; ; ++n) {
        wchar_t ch = n < s ? wLiteral[n] : 0;

        if(wordStart < n) { // inside a word
            if(iswalnum(ch)) {
                // add to current word
            } else if(ch == '.') {
                // add to current word but remember the dot position
                lastDot = n;
            } else { // end of the word

                // re-read current char next time
                --n;
                
                // check for the dot
                if(lastDot < n) {
                    // dot is present but not the last char
                    n = lastDot;
                }

                // extract and lower the word
                bool isAbbr = false;
                size_t lastUpper = string::npos;
                size_t firstLower = string::npos;
                std::wstring word(wLiteral.begin() + wordStart, wLiteral.begin() + n + 1);
                std::wstring word1(wLiteral.begin() + wordStart, wLiteral.begin() + n + 1);

                // reset the word
                size_t pLastDot = lastDot;
                wordStart = lastDot = string::npos;

                for(size_t k = 0; k < word.length(); ++k) {
                    if(iswupper(word1[k])) {
                        word1[k] = towlower(word1[k]);
                        lastUpper = k;
                        if(k != 0) isAbbr = true;
                    } else if(iswlower(word1[k])) {
                        if(firstLower > k) firstLower = k;
                    } else { // digit or dot
                        isAbbr = true;
                    }
                }

                if(n+1 < s && _isIgnoredWord(wStrToStr(word))) {
                    // Skip articles, but not at the end
                    continue;
                }

                // check for abbr in dictionary
                if(!isAbbr && word.length() <= wordsDict->abbrMaxLen()) {
                    isAbbr = !wordsDict->contains(wStrToStr(word));
                    if(isAbbr && lastUpper == 0)
                        isAbbr = !wordsDict->contains(wStrToStr(word1));
                }
                
                // process the word
                if(isAbbr) {
                    if(pLastDot > n) {
                        // Stem plural forms for uppercase abbrevations
                        if(word.length() > 2 && firstLower == word.length()-2 &&
                                word[word.length()-2] == L'e' &&
                                word[word.length()-1] == L's') {
                            word.resize(word.length() - 2);
                        } else if(word.length() > 1 && firstLower == word.length()-1 &&
                                word[word.length()-1] == L's') {
                            word.resize(word.length() - 1);
                        }
                        for(size_t k=0; k<word.length(); ++k) {
                            nWLiteral += iswlower(word[k]) ? towupper(word[k]) : word[k];
                            nWLiteral += (L'.');
                        }
                    } else {
                        for(size_t k=0; k<word.length(); ++k) {
                            nWLiteral += iswlower(word[k]) ? towupper(word[k]) : word[k];
                        }
                    }
                } else {
                    nWLiteral += stem_wstring(word1, multilang);
                }
            }
        } else { // not inside a word
            if(_isIgnored(ch)) {
                continue; // ignore these chars
            } else if(iswalnum(ch)) {
                wordStart = n;
            } else if(ch == '\'' && n+1 < s && (literal[n+1] == 's') &&
                    n != 0 && (iswalpha(literal[n-1]))) {
                if(n+2==s) break;
                wchar_t ch2 = wLiteral[n+2];
                if(iswalnum(ch2) || ch2 == '.')
                    nWLiteral += wLiteral[n];
                else
                    ++n;
            } else if(n >= s) {
                break;
            } else {
                nWLiteral += wLiteral[n]; // use character as-is
            }
        }
    }
    if(postWhiteList.has_key(wStrToStr(nWLiteral, true))) return literal;

    return wStrToStr(nWLiteral);
}

string absolutePath(const string& str, const string& workdir)
{
    using boost::filesystem::path;
    path p = path(str);
    if(!p.is_complete()) {
        if(!workdir.empty()) {
            path w = path(workdir);
            if(!w.is_complete()) 
                w = boost::filesystem::current_path() / w;
            p = w / p;
        } else {
            p = boost::filesystem::current_path() / p;
        }
    }

    string result = p.normalize().string(); 
    if(result.size() > 2 && 0==result.compare(result.size()-2, 2, "/."))
        result.resize(result.size()-2);
    return result;
}

bool isLocalFile(const string& str, const string& workdir)
{
    string aWorkdir = absolutePath(workdir, string());
    return absolutePath(str, aWorkdir)
            .compare(0, aWorkdir.size(), aWorkdir) == 0;
}

#define __PYTHON_NEXT(obj, iter) \
    object obj; \
    try { obj = iter.attr("next")(); } \
    catch (error_already_set const &) { \
        if(PyErr_ExceptionMatches(PyExc_StopIteration)) break; \
        throw; \
    }

struct TextTag
{
    enum Type {
        TT_OTHER = 0,
        TT_WORD, TT_CHARACTER, TT_LITERAL
    };

    Type   type;
    size_t start;
    size_t end;
    string value;
    std::wstring wcvalue;

    // XXX: boost::python does not support pickling of enums
    explicit TextTag(int t = TT_OTHER, size_t s = 0, size_t e = 0,
                            const string& val = string())
        : type(Type(t)), start(s), end(e), value(val), wcvalue(strToWStr(val)) {}

    bool operator==(const TextTag& o) {
        return o.type == type && o.start == start &&
               o.end == end && o.value == value;
    }

    bool operator!=(const TextTag& o) {
        return o.type != type || o.start != start ||
               o.end != end || o.value != value;
    }

    string repr() const {
        static const char* types[] = {
            "OTHER", "WORD", "CHARACTER", "LITERAL"
        };
        std::ostringstream out;
        out << "TextTag("
            << (type <= TT_LITERAL && type >= 0 ? types[type] : "UNKNOWN")
            << ", " << start << ", " << end << ", \"" << value << "\")";
        return out.str();
    }
};

typedef std::vector<TextTag> TextTagList;

string textTagListRepr(const TextTagList& list)
{
    std::ostringstream out;
    out << "TextTagList(";
    TextTagList::const_iterator e = list.end();
    for(TextTagList::const_iterator it = list.begin(); it != e; ++it) {
        if(it != list.begin())
            out << ", ";
        out << it->repr();
    }
    out << ")";
    return out.str();
}

struct TextTagPickeSuite: pickle_suite
{
    static tuple getinitargs(const TextTag& tag) {
        return make_tuple(int(tag.type), tag.start, tag.end, tag.value);
    }
};

struct TextTagListPickeSuite: pickle_suite
{
    static list getstate(const TextTagList& l) {
        list ret; TextTagList::const_iterator e = l.end();
        for(TextTagList::const_iterator it = l.begin(); it != e; ++it) {
            ret.append(make_tuple(int(it->type), it->start, it->end, it->value));
        }
        return ret;
    }
    static void setstate(TextTagList& l, list state) {
        l.resize(len(state));
        size_t n = 0;
        for(stl_input_iterator<tuple> it(state), e; it != e; ++it) {
            int type = extract<int>((*it)[0]);
            size_t start = extract<size_t>((*it)[1]);
            size_t end = extract<size_t>((*it)[2]);
            string value = extract<string>((*it)[3]);
            l[n++] = TextTag(type, start, end, value);
        }
    }
};

void _extractTextInfo(dict& result, const Node::ptr node,
        const boost::regex& exclude_regex,
        const string& workdir = string())
{
    size_t childrenCount = node->childrenCount();

    if(childrenCount == 0) {
        return;
    }

    string aWorkdir = absolutePath(workdir, string());
    shared_ptr<string> lastFile;
    TextTagList* tags = 0;
    for(size_t n = 0; n < childrenCount; ++n) {
        Node::ptr child = node->child(n);

        // check type
        TextTag::Type type;
        if(child->type() == "text_word") {
            type = TextTag::TT_WORD;
        } else if(child->type() == "text_character" ||
                  child->type() == "text_space") {
            type = TextTag::TT_CHARACTER;
        } else {
            type = TextTag::TT_OTHER;
        }

        if(type != TextTag::TT_OTHER) {
            if(child->isOneFile()) {
                shared_ptr<string> file = child->oneFile();
                if(file && (file == lastFile ||
                            isLocalFile(*file, workdir))) {
                    if(file != lastFile || !tags) {
                        lastFile = file;
                        string ffile = absolutePath(*file, aWorkdir)
                                        .substr(aWorkdir.size()+1);
                        TextTagList& tl = extract<TextTagList&>(
                            result.setdefault(ffile, TextTagList()));
                        tags = &tl;
                    }

                    // Save the node
                    std::pair<size_t, size_t> pos = child->sourcePos();
                    tags->push_back(TextTag(type, pos.first, pos.second,
                                                child->valueString()));
                }
            }
        } else if(child->type().compare(0, 12, "environment_") == 0 &&
                    !boost::regex_match(child->type(), exclude_regex)) {
            _extractTextInfo(result, child, exclude_regex, workdir);
            tags = 0; // XXX: it it really required ?
        }
    }
}

dict extractTextInfo(const Node::ptr node, const string& exclude_regex,
                const string& workdir = string())
{
    dict result;
    boost::regex rx(exclude_regex, boost::regex::extended);
    _extractTextInfo(result, node, rx, workdir);
    return result;
}

string getDocumentEncoding(const Node::ptr node)
{
    string result;
    const Node::ChildrenList& c = node->children();
    for(Node::ChildrenList::const_iterator it = c.begin(), e = c.end();
                            it != e; ++it) {
        if(it->second->type() == "inputenc") {
            result = it->second->valueString();
            break;
        }
    }
    if(!result.empty())
        return result;
    else
        return string("ascii");
}

TextTagList findLiterals(const TextTagList& tags,
        const dict& literals, const dict& notLiterals,
        const WordsDict* wordsDict, const dict& preWhiteList,
        const dict& postWhiteList, size_t maxChars = 0, bool multilang=false)
{
    TextTagList result;

    // Detemine a maximum literal length
    if(maxChars == 0) {
        for(stl_input_iterator<str> it(literals.iterkeys()), e;
                                                it != e; ++it) {
            size_t l = len(*it);
            if(l > maxChars)
                maxChars = l;
        }
    }

    // Process the text
    size_t count = tags.size();
    for(size_t n = 0; n < count; ++n) {
        if(tags[n].type == TextTag::TT_CHARACTER) {
            // Do not start from ignored character
            if(_isIgnored(tags[n].wcvalue[0])) continue;
        } else if(tags[n].type != TextTag::TT_WORD) {
            // Ignore unknown tags
            continue;
        }

        // If previous tag is character and is adjacent,
        // then it should be a space
        if(n && tags[n-1].end == tags[n].start &&
                tags[n-1].type == TextTag::TT_CHARACTER &&
                _isglue(tags[n-1].wcvalue[0])) {
            continue;
        }

        // Do not start on an article
        if(_isIgnoredWord(tags[n].value)) {
            continue;
        }

        string text;
        size_t pos = tags[n].start;
        std::vector<boost::tuple<string, size_t, size_t> > foundLiterals;

        for(size_t k = n; k < count; ++k) {
            const TextTag& tagk = tags[k];

            // Stop if tag is not adjacent
            if(tagk.start != pos) {
                break;
            }
            pos = tagk.end;

            text += tagk.value;
            if(tagk.type == TextTag::TT_CHARACTER) {
                // Skip ignored characters
                if(_isIgnored(tagk.wcvalue[0])) continue;
            } else if(tagk.type != TextTag::TT_WORD) {
                // Stop on unknown tags
                break;
            }

            // If next tag is character and is adjacent,
            // then it should be a space
            if(k+1 < count && tags[k+1].start == tags[k].end &&
                    tags[k+1].type == TextTag::TT_CHARACTER &&
                    _isglue(tags[k+1].wcvalue[0])) {
                continue;
            }

            // Norm literal
            string literal = normLiteral(text, wordsDict, preWhiteList, postWhiteList, multilang);
            if(literal.size() > maxChars) {
                continue; // XXX: can normLiteral size gets smaller ?
            }

            // Lookup in dictionary
            if(literals.has_key(literal)) {
                // Skip known non-literal words
                if((!notLiterals.has_key(text)) &&
                        (k+1>=count ||
                         tags[k+1].type != TextTag::TT_CHARACTER ||
                         tags[k+1].wcvalue[0] != L'.' ||
                         !notLiterals.has_key(text+'.'))) {
                    foundLiterals.push_back(
                            boost::make_tuple(literal, tagk.end, k));
                }
            }
        }

        if(!foundLiterals.empty()) { // XXX: return all found literals !
            // Create a tag for the longest literal found
            result.push_back(TextTag(TextTag::TT_LITERAL, tags[n].start,
                                foundLiterals.back().get<1>(),
                                foundLiterals.back().get<0>()));
            n = foundLiterals.back().get<2>();
        }
    }

    return result;
}

string replaceLiterals(const string& source,
                    const TextTagList& tags)
{
    string result;
    result.reserve(source.size() + source.size()/5);
    size_t pos = 0;
    TextTagList::const_iterator end = tags.end();
    for(TextTagList::const_iterator it = tags.begin(); it != end; ++it) {
        if(it->type == TextTag::TT_LITERAL) {
            result += source.substr(pos, it->start - pos);
            result += it->value;
            pos = it->end;
        }
    }
    result += source.substr(pos, source.size() - pos);
    return result;
}

void export_TextTag()
{
    using namespace boost::python;
    scope scope_TextTag = class_<TextTag>("TextTag",
            init<int, size_t, size_t, string>())
        .def_readwrite("type", &TextTag::type)
        .def_readwrite("start", &TextTag::start)
        .def_readwrite("end", &TextTag::end)
        .def_readwrite("value", &TextTag::value)
        .def(self == self)
        .def(self != self)
        .def("__repr__", &TextTag::repr)
        .def_pickle(TextTagPickeSuite())
    ;

    enum_<TextTag::Type>("Type")
        .value("OTHER", TextTag::TT_OTHER)
        .value("WORD", TextTag::TT_WORD)
        .value("CHARACTER", TextTag::TT_CHARACTER)
        .value("LITERAL", TextTag::TT_LITERAL)
    ;
}

BOOST_PYTHON_MODULE(_chrefliterals)
{
    using namespace boost::python;

    export_TextTag();
    class_<TextTagList>("TextTagList")
        .def("__repr__", &textTagListRepr)
        .def(vector_indexing_suite<TextTagList>())
        .def_pickle(TextTagListPickeSuite())
    ;

    class_<WordsDict>("WordsDict", init<string, size_t>())
        .def("insert", &WordsDict::insert)
        .def("contains", &WordsDict::contains)
    ;

    def("absolutePath", &absolutePath);
    def("isLocalFile", &isLocalFile);
    def("normLiteral", &normLiteral);
    def("extractTextInfo", &extractTextInfo);
    def("getDocumentEncoding", &getDocumentEncoding);
    def("findLiterals", &findLiterals);
    def("replaceLiterals", &replaceLiterals);
}

