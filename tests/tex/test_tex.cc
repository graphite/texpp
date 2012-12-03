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

#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

using namespace std;

const char* tex_executable;
const char* texpp_executable;

#ifndef WINDOWS
#define PATH_SEP '/'
#define DEV_NULL "/dev/null"
#else
#define PATH_SEP '\\'
#define DEV_NULL "/dev/null"
#endif

void test_usage()
{
    BOOST_FAIL( "Wrong number of command line arguments\n"
                "Usage: test_tex <tex_executable>"
                " <texpp_executable> file1 file2 ..." );
}

vector<string> read_log_file(const string& fname)
{
    vector<string> res;
    ifstream file(fname.c_str());

    BOOST_REQUIRE_MESSAGE( file.good(), "Can not open output file");

    char buf[1024]; int n = 0;
    while(file.good()) {
        buf[0] = 0;
        file.getline(buf, sizeof(buf));

        if(buf[0] == '>' && std::strlen(buf) >= 9 &&
                    std::strcmp(buf+std::strlen(buf)-6, "macro:") == 0) {
            n = 2;
        } else if(buf[0] == 'l' && buf[1] == '.') {
            n = 2;
        } else if(buf[0] == '>' || buf[0] == '!' ||
                    buf[0] == '~' || buf[0] == '{') {
            n = 1;
        } else if(buf[0] == '#' && std::isdigit(buf[1]) &&
                  buf[2] == '<' && buf[3] == '-') {
            n = 1;
        } else if(std::strncmp(buf, "\\testmacro", 10) == 0) {
            n = 1;
        } else if(std::strncmp(buf, "\\write", 6) == 0) {
            n = 1;
        } else if(std::strncmp(buf, "\\openout", 8) == 0) {
            n = 1;
        }

        if(n) {
            res.push_back(string(buf));
            --n;
        }

        if(buf[0] == '{' && std::strlen(buf) >= 6 &&
                    std::strcmp(buf+std::strlen(buf)-5, "\\end}") == 0) {
            break;
        }
    }

    return res;
}

void test_tex(const char* testfile)
{
    string jobname(testfile);

    size_t n = jobname.rfind(PATH_SEP);
    if(n != jobname.npos)
        jobname = jobname.substr(n+1);
    n = jobname.rfind(".tex");
    if(n != jobname.npos)
        jobname = jobname.substr(0, n);

    string log_tex = jobname + ".log";
    string log_texpp = jobname + ".log.pp";

    string cmd1 = string(tex_executable)
                    + " -ini -interaction nonstopmode"
                    + " '" + testfile + "' > " + DEV_NULL;

    string cmd2 = string(texpp_executable)
                    + " '" + testfile + "' > '" + log_texpp + "'";

    system(cmd1.c_str());
    system(cmd2.c_str());

    vector<string> log_tex_l = read_log_file(log_tex);
    vector<string> log_texpp_l = read_log_file(log_texpp);

    if(jobname.substr(0, 5) == "fail_") {
        BOOST_CHECK(log_tex_l != log_texpp_l);
    } else {
        BOOST_CHECK_EQUAL_COLLECTIONS(log_tex_l.begin(), log_tex_l.end(),
                              log_texpp_l.begin(), log_texpp_l.end());
    }
}

boost::unit_test::test_suite*
init_unit_test_suite( int argc, char* argv[] ) 
{
    if(argc < 3) {
        boost::unit_test::framework::master_test_suite().
            add( BOOST_TEST_CASE( &test_usage ) );
        return 0;
    }

    tex_executable = argv[1];
    texpp_executable = argv[2];

    boost::unit_test::framework::master_test_suite().
        add( BOOST_PARAM_TEST_CASE( &test_tex, argv+3, argv+argc ) );

    return 0;
}

