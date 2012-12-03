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

#include <texpp/base/base.h>

#include <texpp/base/conditional.h>
#include <texpp/base/miscmacros.h>
#include <texpp/base/misc.h>
#include <texpp/base/show.h>
#include <texpp/base/func.h>
#include <texpp/base/files.h>
#include <texpp/base/variable.h>
#include <texpp/base/integer.h>
#include <texpp/base/dimen.h>
#include <texpp/base/glue.h>
#include <texpp/base/toks.h>
#include <texpp/base/font.h>
#include <texpp/base/char.h>
#include <texpp/base/parshape.h>
#include <texpp/base/hyphenation.h>
#include <texpp/base/box.h>

#include <texpp/parser.h>

#include <boost/lexical_cast.hpp>
#include <ctime>
#include <climits>

namespace texpp {
namespace base {

void initSymbols(Parser& parser)
{
    #define __TEXPP_SET_COMMAND(name, T, ...) \
        parser.setSymbol("\\" name, \
            Command::ptr(new T("\\" name, ##__VA_ARGS__)))

    // conditionals
    __TEXPP_SET_COMMAND("iftrue", Iftrue);
    __TEXPP_SET_COMMAND("iffalse", Iffalse);

    __TEXPP_SET_COMMAND("ifnum", Ifnum);
    __TEXPP_SET_COMMAND("ifdim", Ifdim);
    __TEXPP_SET_COMMAND("ifodd", Ifodd);

    __TEXPP_SET_COMMAND("ifvmode", Ifmode,
            (1<<Parser::VERTICAL) | (1<<Parser::RVERTICAL));
    __TEXPP_SET_COMMAND("ifhmode", Ifmode,
            (1<<Parser::HORIZONTAL) | (1<<Parser::RHORIZONTAL));
    __TEXPP_SET_COMMAND("ifmmode", Ifmode,
            (1<<Parser::MATH) | (1<<Parser::DMATH));
    __TEXPP_SET_COMMAND("ifinner", Ifmode,
            (1<<Parser::RVERTICAL) | (1<<Parser::RHORIZONTAL) |
            (1<<Parser::MATH));

    __TEXPP_SET_COMMAND("ifcat", Ifcat);
    __TEXPP_SET_COMMAND("if", Ifcat, false);

    __TEXPP_SET_COMMAND("ifx", Ifx);

    __TEXPP_SET_COMMAND("ifvoid", Ifvoid);
    __TEXPP_SET_COMMAND("ifhbox", Ifhbox);
    __TEXPP_SET_COMMAND("ifvbox", Ifvbox);

    __TEXPP_SET_COMMAND("ifcase", Ifcase);
    
    __TEXPP_SET_COMMAND("ifeof", Ifeof);

    __TEXPP_SET_COMMAND("or", ConditionalOr);
    __TEXPP_SET_COMMAND("else", ConditionalElse);
    __TEXPP_SET_COMMAND("fi", ConditionalEnd);

    // macros
    __TEXPP_SET_COMMAND("number", NumberMacro);
    __TEXPP_SET_COMMAND("romannumeral", RomannumeralMacro);
    __TEXPP_SET_COMMAND("string", StringMacro);
    __TEXPP_SET_COMMAND("meaning", MeaningMacro);
    __TEXPP_SET_COMMAND("the", TheMacro);

    __TEXPP_SET_COMMAND("expandafter", ExpandafterMacro);
    __TEXPP_SET_COMMAND("noexpand", NoexpandMacro);

    __TEXPP_SET_COMMAND("fontname", FontnameMacro);
    __TEXPP_SET_COMMAND("jobname", JobnameMacro);

    __TEXPP_SET_COMMAND("csname", CsnameMacro);
    __TEXPP_SET_COMMAND("endcsname", EndcsnameMacro);

    // I/O
    __TEXPP_SET_COMMAND("openin", Openin);
    __TEXPP_SET_COMMAND("closein", Closein);
    __TEXPP_SET_COMMAND("read", Read);

    __TEXPP_SET_COMMAND("input", Input);
    __TEXPP_SET_COMMAND("endinput", Endinput);

    __TEXPP_SET_COMMAND("immediate", Immediate);
    __TEXPP_SET_COMMAND("openout", Openout);
    __TEXPP_SET_COMMAND("closeout", Closeout);
    __TEXPP_SET_COMMAND("write", Write);

    __TEXPP_SET_COMMAND("message", Message);
    
    // various commands
    __TEXPP_SET_COMMAND("end", End);
    __TEXPP_SET_COMMAND("par", Par);

    __TEXPP_SET_COMMAND("relax", Relax);
    parser.setSymbol("relax", parser.symbolAny("\\relax"));

    __TEXPP_SET_COMMAND("uppercase", Changecase, "uccode");
    __TEXPP_SET_COMMAND("lowercase", Changecase, "lccode");

    __TEXPP_SET_COMMAND("let", Let);
    __TEXPP_SET_COMMAND("futurelet", Futurelet);

    __TEXPP_SET_COMMAND("def", Def);
    __TEXPP_SET_COMMAND("gdef", Def, true);
    __TEXPP_SET_COMMAND("edef", Def, false, true);
    __TEXPP_SET_COMMAND("xdef", Def, true, true);

    __TEXPP_SET_COMMAND("show", Show);
    __TEXPP_SET_COMMAND("showthe", ShowThe);

    __TEXPP_SET_COMMAND("begingroup", Begingroup);
    __TEXPP_SET_COMMAND("endgroup", Endgroup);

    // TODO: implement everypar etc.
    __TEXPP_SET_COMMAND("afterassignment", Afterassignment);
    __TEXPP_SET_COMMAND("aftergroup", Aftergroup);

    // horizontal mode
    __TEXPP_SET_COMMAND("vrule", Rule, Parser::VERTICAL);

    // vertical mode
    __TEXPP_SET_COMMAND("hrule", Rule, Parser::HORIZONTAL);

    // prefixes
    __TEXPP_SET_COMMAND("global", Prefix);
    __TEXPP_SET_COMMAND("long", Prefix);
    __TEXPP_SET_COMMAND("outer", Prefix);

    __TEXPP_SET_COMMAND("advance",
        ArithmeticCommand, Variable::ADVANCE);
    __TEXPP_SET_COMMAND("multiply",
        ArithmeticCommand, Variable::MULTIPLY);
    __TEXPP_SET_COMMAND("divide",
        ArithmeticCommand, Variable::DIVIDE);

    __TEXPP_SET_COMMAND("hyphenation", Hyphenation);
    __TEXPP_SET_COMMAND("patterns", Hyphenation);

    __TEXPP_SET_COMMAND("hbox", BoxSpec, Parser::RHORIZONTAL, false);
    __TEXPP_SET_COMMAND("vbox", BoxSpec, Parser::RVERTICAL, false);
    __TEXPP_SET_COMMAND("vtop", BoxSpec, Parser::RVERTICAL, true);

    __TEXPP_SET_COMMAND("box", Register<BoxVariable>, Box());
    __TEXPP_SET_COMMAND("copy", Register<BoxVariable>, Box());
    __TEXPP_SET_COMMAND("vsplit", Vsplit);
    __TEXPP_SET_COMMAND("lastbox", Lastbox);

    __TEXPP_SET_COMMAND("setbox", Setbox);

    __TEXPP_SET_COMMAND("nullfont", FontSelector, base::defaultFontInfo);

    #define __TEXPP_SET_FONT_GROUP(name, T) \
        __TEXPP_SET_COMMAND(name, T, base::defaultFontInfo); \
        parser.setSymbol(name, base::defaultFontInfo)

    __TEXPP_SET_FONT_GROUP("font", Font);
    __TEXPP_SET_FONT_GROUP("textfont", FontFamily);
    __TEXPP_SET_FONT_GROUP("scriptfont", FontFamily);
    __TEXPP_SET_FONT_GROUP("scriptscriptfont", FontFamily);

    __TEXPP_SET_COMMAND("hyphenchar", FontChar);
    __TEXPP_SET_COMMAND("skewchar", FontChar);

    __TEXPP_SET_COMMAND("fontdimen", FontDimen);

    __TEXPP_SET_COMMAND("catcode", CharcodeVariable, int(0), 0, 15);
    __TEXPP_SET_COMMAND("lccode", CharcodeVariable, int(0), 0, 255);
    __TEXPP_SET_COMMAND("uccode", CharcodeVariable, int(0), 0, 255);
    __TEXPP_SET_COMMAND("sfcode", CharcodeVariable, int(0), 0, 32767);
    __TEXPP_SET_COMMAND("mathcode", CharcodeVariable, int(0), 0, 32768);
    __TEXPP_SET_COMMAND("delcode", CharcodeVariable, int(0),
                                        TEXPP_INT_MIN, 16777215);

    #define __TEXPP_SET_REGISTER(name, T, v) \
        __TEXPP_SET_COMMAND(name, Register<T>, v); \
        parser.setSymbol("\\" name "def", \
            Command::ptr(new RegisterDef<Register<T> >("\\" name "def", \
                static_pointer_cast<Register<T> >( \
                    parser.symbol("\\" name, Command::ptr())))))

    __TEXPP_SET_REGISTER("count", IntegerVariable, int(0));

    __TEXPP_SET_REGISTER("dimen", DimenVariable, Dimen(0));
    __TEXPP_SET_REGISTER("ht", BoxDimen, Dimen(0));
    __TEXPP_SET_REGISTER("wd", BoxDimen, Dimen(0));
    __TEXPP_SET_REGISTER("dp", BoxDimen, Dimen(0));

    __TEXPP_SET_REGISTER("skip", GlueVariable, Glue(0,0));
    __TEXPP_SET_REGISTER("muskip", MuGlueVariable, Glue(1,0));
    __TEXPP_SET_REGISTER("toks", ToksVariable, Token::list());

    #define __TEXPP_SET_CHAR(name, T) \
        __TEXPP_SET_COMMAND(name, T); \
        parser.setSymbol("\\" name "def", \
            Command::ptr(new RegisterDef<T>("\\" name "def", \
                static_pointer_cast<T>( \
                    parser.symbol("\\" name, Command::ptr())))))

    __TEXPP_SET_CHAR("char", Char);
    __TEXPP_SET_CHAR("mathchar", MathChar);
    __TEXPP_SET_COMMAND("delimiter", Delimiter);

    #define __TEXPP_SET_VARIABLE(name, T, value, ...) \
        __TEXPP_SET_COMMAND(name, T, value, ##__VA_ARGS__); \
        parser.setSymbol(name, value)

    __TEXPP_SET_VARIABLE("parshape", Parshape, ParshapeInfo());

    __TEXPP_SET_VARIABLE("lastpenalty",
            ReadOnlyVariable<InternalInteger>, int(0));
    __TEXPP_SET_VARIABLE("inputlineno",
            ReadOnlyVariable<InternalInteger>, int(0));
    __TEXPP_SET_VARIABLE("badness",
            ReadOnlyVariable<InternalInteger>, int(0));

    __TEXPP_SET_VARIABLE("lastkern",
            ReadOnlyVariable<InternalDimen>, Dimen(0));

    __TEXPP_SET_VARIABLE("lastskip",
            ReadOnlyVariable<InternalGlue>, Glue(0,0));

    __TEXPP_SET_VARIABLE("endlinechar", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("mag", IntegerVariable, int(0));

    __TEXPP_SET_VARIABLE("pretolerance", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tolerance", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("hbadness", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("vbadness", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("linepenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("hyphenpenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("exhyphenpenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("binoppenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("relpenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("clubpenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("widowpenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("displaywidowpenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("brokenpenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("predisplaypenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("postdisplaypenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("interlinepenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("floatingpenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("outputpenalty", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("doublehyphendemerits", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("finalhyphendemerits", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("adjdemerits", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("looseness", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("pausing", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("holdinginserts", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tracingonline", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tracingmacros", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tracingstats", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tracingparagraphs", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tracingpages", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tracingoutput", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tracinglostchars", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tracingcommands", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("tracingrestores", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("language", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("uchyph", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("lefthyphenmin", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("righthyphenmin", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("globaldefs", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("defaulthyphenchar", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("defaultskewchar", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("escapechar", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("newlinechar", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("maxdeadcycles", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("hangafter", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("fam", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("delimiterfactor", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("time", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("day", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("month", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("year", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("showboxbreadth", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("showboxdepth", IntegerVariable, int(0));
    __TEXPP_SET_VARIABLE("errorcontextlines", IntegerVariable, int(0));

    __TEXPP_SET_VARIABLE("hfuzz", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("vfuzz", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("overfullrule", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("emergencystretch", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("hsize", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("vsize", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("maxdepth", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("splitmaxdepth", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("boxmaxdepth", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("lineskiplimit", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("delimitershortfall", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("nulldelimiterspace", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("scriptspace", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("mathsurround", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("predisplaysize", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("displaywidth", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("displayindent", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("parindent", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("hangindent", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("hoffset", DimenVariable, Dimen(0));
    __TEXPP_SET_VARIABLE("voffset", DimenVariable, Dimen(0));

    __TEXPP_SET_VARIABLE("baselineskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("lineskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("parskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("abovedisplayskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("abovedisplayshortskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("belowdisplayskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("belowdisplayshortskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("leftskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("rightskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("topskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("splittopskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("tabskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("spaceskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("xspaceskip", GlueVariable, Glue(0,0));
    __TEXPP_SET_VARIABLE("parfillskip", GlueVariable, Glue(0,0));

    __TEXPP_SET_VARIABLE("thinmuskip", MuGlueVariable, Glue(1,0));
    __TEXPP_SET_VARIABLE("medmuskip", MuGlueVariable, Glue(1,0));
    __TEXPP_SET_VARIABLE("thickmuskip", MuGlueVariable, Glue(1,0));

    __TEXPP_SET_VARIABLE("output", ToksVariable, Token::list());
    __TEXPP_SET_VARIABLE("everypar", ToksVariable, Token::list());
    __TEXPP_SET_VARIABLE("everymath", ToksVariable, Token::list());
    __TEXPP_SET_VARIABLE("everydisplay", ToksVariable, Token::list());
    __TEXPP_SET_VARIABLE("everyhbox", ToksVariable, Token::list());
    __TEXPP_SET_VARIABLE("everyvbox", ToksVariable, Token::list());
    __TEXPP_SET_VARIABLE("everyjob", ToksVariable, Token::list());
    __TEXPP_SET_VARIABLE("everycr", ToksVariable, Token::list());
    __TEXPP_SET_VARIABLE("errhelp", ToksVariable, Token::list());

    // special
    __TEXPP_SET_COMMAND("spacefactor", Spacefactor, int(0));
    __TEXPP_SET_COMMAND("prevgraf", SpecialInteger, int(0));
    __TEXPP_SET_COMMAND("deadcycles", SpecialInteger, int(0));
    __TEXPP_SET_COMMAND("insertpenalties", SpecialInteger, int(0));

    __TEXPP_SET_COMMAND("prevdepth", SpecialDimen, Dimen(0));
    __TEXPP_SET_COMMAND("pagegoal", SpecialDimen, Dimen(0));
    __TEXPP_SET_COMMAND("pagetotal", SpecialDimen, Dimen(0));
    __TEXPP_SET_COMMAND("pagestretch", SpecialDimen, Dimen(0));
    __TEXPP_SET_COMMAND("pagefilstrerch", SpecialDimen, Dimen(0));
    __TEXPP_SET_COMMAND("pagefillstrerch", SpecialDimen, Dimen(0));
    __TEXPP_SET_COMMAND("pagefilllstrerch", SpecialDimen, Dimen(0));
    __TEXPP_SET_COMMAND("pageshrink", SpecialDimen, Dimen(0));
    __TEXPP_SET_COMMAND("pagedepth", SpecialDimen, Dimen(0));

    // interaction
    __TEXPP_SET_COMMAND("errorstopmode", SetInteraction,
                                            Parser::ERRORSTOPMODE);
    __TEXPP_SET_COMMAND("scrollmode", SetInteraction, Parser::SCROLLMODE);
    __TEXPP_SET_COMMAND("nonstopmode", SetInteraction, Parser::NONSTOPMODE);
    __TEXPP_SET_COMMAND("batchmode", SetInteraction, Parser::BATCHMODE);

    // Ignored commands
    __TEXPP_SET_COMMAND("dump", IgnoredCommand);

    // Unimplemented commands

    __TEXPP_SET_COMMAND("spread", UnimplementedCommand);

    __TEXPP_SET_COMMAND("showbox", UnimplementedCommand);
    __TEXPP_SET_COMMAND("showlists", UnimplementedCommand);
    __TEXPP_SET_COMMAND("shipout", UnimplementedCommand);
    __TEXPP_SET_COMMAND("ignorespaces", UnimplementedCommand);
    __TEXPP_SET_COMMAND("errmessage", UnimplementedCommand);
    __TEXPP_SET_COMMAND("special", UnimplementedCommand);
    __TEXPP_SET_COMMAND("penalty", UnimplementedCommand);
    __TEXPP_SET_COMMAND("kern", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mkern", UnimplementedCommand);
    __TEXPP_SET_COMMAND("unpenalty", UnimplementedCommand);
    __TEXPP_SET_COMMAND("unkern", UnimplementedCommand);
    __TEXPP_SET_COMMAND("unskip", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mark", UnimplementedCommand);
    __TEXPP_SET_COMMAND("insert", UnimplementedCommand);
    __TEXPP_SET_COMMAND("vadjust", UnimplementedCommand);

    // vertical mode
    __TEXPP_SET_COMMAND("vskip", UnimplementedCommand);
    __TEXPP_SET_COMMAND("vfil", UnimplementedCommand);
    __TEXPP_SET_COMMAND("vfill", UnimplementedCommand);
    __TEXPP_SET_COMMAND("vss", UnimplementedCommand);
    __TEXPP_SET_COMMAND("vfilneg", UnimplementedCommand);
    __TEXPP_SET_COMMAND("leaders", UnimplementedCommand);
    __TEXPP_SET_COMMAND("cleaders", UnimplementedCommand);
    __TEXPP_SET_COMMAND("xleaders", UnimplementedCommand);
    __TEXPP_SET_COMMAND("moveleft", UnimplementedCommand);
    __TEXPP_SET_COMMAND("moveright", UnimplementedCommand);
    __TEXPP_SET_COMMAND("unvbox", UnimplementedCommand);
    __TEXPP_SET_COMMAND("unvcopy", UnimplementedCommand);
    __TEXPP_SET_COMMAND("halign", UnimplementedCommand);
    __TEXPP_SET_COMMAND("indent", UnimplementedCommand);
    __TEXPP_SET_COMMAND("noindent", UnimplementedCommand);

    // horizontal mode
    __TEXPP_SET_COMMAND("hskip", UnimplementedCommand);
    __TEXPP_SET_COMMAND("hfil", UnimplementedCommand);
    __TEXPP_SET_COMMAND("hfill", UnimplementedCommand);
    __TEXPP_SET_COMMAND("hss", UnimplementedCommand);
    __TEXPP_SET_COMMAND("hfilneg", UnimplementedCommand);
    __TEXPP_SET_COMMAND("raise", UnimplementedCommand);
    __TEXPP_SET_COMMAND("lower", UnimplementedCommand);
    __TEXPP_SET_COMMAND("unhbox", UnimplementedCommand);
    __TEXPP_SET_COMMAND("unhcopy", UnimplementedCommand);
    __TEXPP_SET_COMMAND("valign", UnimplementedCommand);
    __TEXPP_SET_COMMAND("accent", UnimplementedCommand);
    __TEXPP_SET_COMMAND("/", UnimplementedCommand);
    __TEXPP_SET_COMMAND("discretionary", UnimplementedCommand);
    __TEXPP_SET_COMMAND("-", UnimplementedCommand);
    __TEXPP_SET_COMMAND("setlanguage", UnimplementedCommand);

    // math mode
    __TEXPP_SET_COMMAND("mskip", UnimplementedCommand);
    __TEXPP_SET_COMMAND("nonscript", UnimplementedCommand);
    __TEXPP_SET_COMMAND("noboundary", UnimplementedCommand);
    __TEXPP_SET_COMMAND("vcenter", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathord", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathop", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathbin", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathrel", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathopen", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathclose", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathpunct", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathinner", UnimplementedCommand);
    __TEXPP_SET_COMMAND("underline", UnimplementedCommand);
    __TEXPP_SET_COMMAND("overline", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathaccent", UnimplementedCommand);
    __TEXPP_SET_COMMAND("radical", UnimplementedCommand);
    __TEXPP_SET_COMMAND("displaylimits", UnimplementedCommand);
    __TEXPP_SET_COMMAND("limits", UnimplementedCommand);
    __TEXPP_SET_COMMAND("nolimits", UnimplementedCommand);
    __TEXPP_SET_COMMAND("mathchoice", UnimplementedCommand);
    __TEXPP_SET_COMMAND("displaystyle", UnimplementedCommand);
    __TEXPP_SET_COMMAND("textstyle", UnimplementedCommand);
    __TEXPP_SET_COMMAND("scriptstyle", UnimplementedCommand);
    __TEXPP_SET_COMMAND("scriptscriptstyle", UnimplementedCommand);
    __TEXPP_SET_COMMAND("left", UnimplementedCommand);
    __TEXPP_SET_COMMAND("right", UnimplementedCommand);
    __TEXPP_SET_COMMAND("over", UnimplementedCommand);
    __TEXPP_SET_COMMAND("atop", UnimplementedCommand);
    __TEXPP_SET_COMMAND("above", UnimplementedCommand);
    __TEXPP_SET_COMMAND("overwithdelims", UnimplementedCommand);
    __TEXPP_SET_COMMAND("atopwithdelims", UnimplementedCommand);
    __TEXPP_SET_COMMAND("abovewithdelims", UnimplementedCommand);

    // unimplemented macros
    __TEXPP_SET_COMMAND("topmark", UnimplementedCommand);
    __TEXPP_SET_COMMAND("firstmark", UnimplementedCommand);
    __TEXPP_SET_COMMAND("botmark", UnimplementedCommand);
    __TEXPP_SET_COMMAND("splitfirstmark", UnimplementedCommand);
    __TEXPP_SET_COMMAND("splitbotmark", UnimplementedCommand);

    // INITEX context
    for(int i=0; i<256; ++i) {
        string n = boost::lexical_cast<string>(i);

        parser.lexer()->setCatcode(i, Token::CC_OTHER);
        parser.setSymbol("catcode"+n, int(Token::CC_OTHER));
        parser.setSymbol("sfcode"+n, int(1000));

        parser.setSymbol("delcode"+n, int(-1));
        parser.setSymbol("mathcode"+n, int(i));
    }

    for(int i='a'; i<='z'; ++i) {
        string n = boost::lexical_cast<string>(i);

        parser.lexer()->setCatcode(i, Token::CC_LETTER);
        parser.setSymbol("catcode"+n, int(Token::CC_LETTER));

        parser.setSymbol("lccode"+n, int(i));
        parser.setSymbol("uccode"+n, int(i - 'a' + 'A'));
        parser.setSymbol("mathcode"+n, int(0x7100 + i));
    }

    for(int i='A'; i<='Z'; ++i) {
        string n = boost::lexical_cast<string>(i);

        parser.lexer()->setCatcode(i, Token::CC_LETTER);
        parser.setSymbol("catcode"+n, int(Token::CC_LETTER));

        parser.setSymbol("sfcode"+n, int(999));
        parser.setSymbol("lccode"+n, int(i + 'a' - 'A'));
        parser.setSymbol("uccode"+n, int(i));
        parser.setSymbol("mathcode"+n, int(0x7100 + i));
    }

    for(int i='0'; i<='9'; ++i) {
        string n = boost::lexical_cast<string>(i);
        parser.setSymbol("mathcode"+n, int(0x7000 + i));
    }

    parser.lexer()->setCatcode(0x7f,   Token::CC_INVALID);
    parser.setSymbol("catcode127", int(Token::CC_INVALID));
    parser.lexer()->setCatcode('\\',   Token::CC_ESCAPE);
    parser.setSymbol("catcode92",  int(Token::CC_ESCAPE));
    parser.lexer()->setCatcode('\r',   Token::CC_EOL);
    parser.setSymbol("catcode13",  int(Token::CC_EOL));
    parser.lexer()->setCatcode(' ',    Token::CC_SPACE);
    parser.setSymbol("catcode32",  int(Token::CC_SPACE));
    parser.lexer()->setCatcode('%',    Token::CC_COMMENT);
    parser.setSymbol("catcode37",  int(Token::CC_COMMENT));

    parser.setSymbol("delcode96", int(0));

    parser.lexer()->setEndlinechar('\r');
    parser.setSymbol("endlinechar", int('\r'));

    parser.setSymbol("escapechar", int('\\'));
    parser.setSymbol("tolerance", int(10000));
    parser.setSymbol("mag", int(1000));
    parser.setSymbol("maxdeadcycles", int(25));

    std::time_t t; std::time(&t);
    std::tm* time = std::localtime(&t);
    parser.setSymbol("year", int(1900+time->tm_year));
    parser.setSymbol("month", int(1+time->tm_mon));
    parser.setSymbol("day", int(time->tm_mday));
    parser.setSymbol("time", int(time->tm_hour*60 + time->tm_min));

    parser.setSymbol("hangafter", int(1));
}

} // namespace base
} // namespace texpp

