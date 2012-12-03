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

extern "C" {
struct stemmer;

extern struct stemmer * create_stemmer(void);
extern void free_stemmer(struct stemmer * z);

extern int stem(struct stemmer * z, char * b, int k);
}

using namespace boost::python;
using namespace texpp;

class Stemmer {
public:
    Stemmer() { _stemmer = create_stemmer(); }
    ~Stemmer() { free_stemmer(_stemmer); }

    string stem(string word) const
    {
        /* TODO: remove unnessesary conversion to/from std::string */
        int n = word.size();
        char buf[n];
        std::memcpy(buf, word.data(), n);
        n = ::stem(_stemmer, buf, n-1);
        return string(buf, n+1);
    }

protected:
    mutable struct stemmer* _stemmer;
};

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

inline bool _islower(char ch) { return ch >= 'a' && ch <= 'z'; }
inline bool _isupper(char ch) { return ch >= 'A' && ch <= 'Z'; }

inline bool _isglue(char ch) {
    return isdigit(ch) || ch == '-' || ch == '/';
}

inline bool _isIgnored(char ch) {
    return ch == ' ' || ch == '~' || ch == '-' || ch == '/';
}

inline bool _isIgnoredWord(const string& word) {
    return word == "the" || word == "a" || word == "an" ||
           word == "The" || word == "A" || word == "An";
}

string normLiteral(string literal,
        const WordsDict* wordsDict, const Stemmer* stemmer, const dict& whiteList)
{
    /* TODO: support arbitrary stemmers and wordDicts */
    /* TODO: unicode and locales */
    string nliteral;
    size_t n = 0, s = literal.size();
    size_t wordStart = string::npos;
    size_t lastDot = string::npos;
    string lowercaseLiteral(literal);
    for(size_t i = 0; i < lowercaseLiteral.size(); ++i)
        if(_isupper(lowercaseLiteral[i])) lowercaseLiteral[i] -= 'A'-'a';
    if(whiteList.has_key(lowercaseLiteral)) return literal;

    /* TODO: handle 's */
    for(n=0; ; ++n) {
        char ch = n < s ? literal[n] : 0;

        if(wordStart < n) { // inside a word
            if(_islower(ch) || _isupper(ch) || std::isdigit(ch)) {
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
                size_t wordSize = n + 1 - wordStart;
                std::string word(literal, wordStart, wordSize);
                std::string word1(literal, wordStart, wordSize);

                // reset the word
                size_t pLastDot = lastDot;
                wordStart = lastDot = string::npos;

                for(size_t k = 0; k < wordSize; ++k) {
                    if(_isupper(word1[k])) {
                        word1[k] += ('a' - 'A');
                        lastUpper = k;
                        if(k != 0) isAbbr = true;
                    } else if(_islower(word1[k])) {
                        if(firstLower > k) firstLower = k;
                    } else { // digit or dot
                        isAbbr = true;
                    }
                }

                if(n+1 < s && _isIgnoredWord(word)) {
                    // Skip articles, but not at the end
                    continue;
                }

                // check for abbr in dictionary
                if(!isAbbr && wordSize <= wordsDict->abbrMaxLen()) {
                    isAbbr = !wordsDict->contains(word);
                    if(isAbbr && lastUpper == 0)
                        isAbbr = !wordsDict->contains(word1);
                }
                
                // process the word
                if(isAbbr) {
                    if(pLastDot > n) {
                        // Stem plural forms for uppercase abbrevations
                        if(wordSize > 2 && firstLower == wordSize-2 &&
                                word[wordSize-2] == 'e' &&
                                word[wordSize-1] == 's') {
                            --wordSize; --wordSize;
                            word.resize(wordSize);
                        } else if(wordSize > 1 && firstLower == wordSize-1 &&
                                word[wordSize-1] == 's') {
                            word.resize(--wordSize);
                        }
                        size_t nliteralSize = nliteral.size();
                        nliteral.resize(nliteralSize + 2*wordSize);
                        for(size_t k=0; k<wordSize; ++k) {
                            nliteral[nliteralSize + (k<<1)] =
                                _islower(word[k]) ?
                                    word[k] - ('a' - 'A') : word[k];
                            nliteral[nliteralSize + (k<<1) + 1] = '.';
                        }
                    } else {
                        size_t nliteralSize = nliteral.size();
                        nliteral.resize(nliteralSize + wordSize);
                        for(size_t k=0; k<wordSize; ++k) {
                            nliteral[nliteralSize + k] =
                                _islower(word[k]) ?
                                    word[k] - ('a' - 'A') : word[k];
                        }
                    }
                } else {
                    nliteral += stemmer->stem(word1);
                }
            }
        } else { // not inside a word
            if(_isIgnored(ch)) {
                continue; // ignore these chars
            } else if(_islower(ch) || _isupper(ch) || std::isdigit(ch)) {
                wordStart = n;
            } else if(ch == '\'' && n+1 < s && (literal[n+1] == 's') &&
                    n != 0 && (_islower(literal[n-1]) ||
                               _isupper(literal[n-1]))) {
                if(n+2==s) break;
                char ch2 = literal[n+2];
                if(_islower(ch2) || _isupper(ch2) || std::isdigit(ch2)
                                                    || ch2 == '.')
                    nliteral += literal[n];
                else
                    ++n;
            } else if(n >= s) {
                break;
            } else {
                nliteral += literal[n]; // use character as-is
            }
        }
    }

    return nliteral;
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

    // XXX: boost::python does not support pickling of enums
    explicit TextTag(int t = TT_OTHER, size_t s = 0, size_t e = 0,
                            const string& val = string())
        : type(Type(t)), start(s), end(e), value(val) {}

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
        const WordsDict* wordsDict, const Stemmer* stemmer, const dict& whiteList,
        size_t maxChars = 0)
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
            if(_isIgnored(tags[n].value[0])) continue;
        } else if(tags[n].type != TextTag::TT_WORD) {
            // Ignore unknown tags
            continue;
        }

        // If previous tag is character and is adjacent,
        // then it should be a space
        if(n && tags[n-1].end == tags[n].start &&
                tags[n-1].type == TextTag::TT_CHARACTER &&
                _isglue(tags[n-1].value[0])) {
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
                if(_isIgnored(tagk.value[0])) continue;
            } else if(tagk.type != TextTag::TT_WORD) {
                // Stop on unknown tags
                break;
            }

            // If next tag is character and is adjacent,
            // then it should be a space
            if(k+1 < count && tags[k+1].start == tags[k].end &&
                    tags[k+1].type == TextTag::TT_CHARACTER &&
                    _isglue(tags[k+1].value[0])) {
                continue;
            }

            // Norm literal
            string literal = normLiteral(text, wordsDict, stemmer, whiteList);
            if(literal.size() > maxChars) {
                continue; // XXX: can normLiteral size gets smaller ?
            }

            // Lookup in dictionary
            if(literals.has_key(literal)) {
                // Skip known non-literal words
                if((!notLiterals.has_key(text)) &&
                        (k+1>=count ||
                         tags[k+1].type != TextTag::TT_CHARACTER ||
                         tags[k+1].value[0] != '.' ||
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

    class_<Stemmer>("Stemmer", init<>())
        .def("stem", &Stemmer::stem)
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

