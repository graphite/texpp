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

#include "kpsewhich.h"
#include "common.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

namespace texpp {

std::string kpseextend(const std::string& fname)
{
    size_t n = fname.rfind(PATH_SEP);
    size_t n1 = fname.substr(n == fname.npos ? 0 : n).rfind('.');
    return n1 == fname.npos ? (fname + ".tex") : fname;
}

std::string kpsewhich(const std::string& fname, const std::string& dir)
{
    int p_stdout[2];
    pid_t pid;

    if(pipe(p_stdout) != 0) {
        return std::string();
    }

    pid = fork();
    if(pid < 0) {
        close(p_stdout[0]);
        close(p_stdout[1]);
        return std::string();
    } else if(pid == 0) {
        close(p_stdout[0]);
        if(p_stdout[1] != 1) {
            dup2(p_stdout[1], 1);
            close(p_stdout[1]);
        }
        if(!dir.empty())
            chdir(dir.c_str());
        execlp("kpsewhich", "kpsewhich", fname.c_str(), NULL);
        exit(1);
    }

    close(p_stdout[1]);

    std::string fullname;
    char buf[256];
    int len;
    while(true) {
        len = read(p_stdout[0], buf, sizeof(buf));
        if(len == 0) {
            break;
        } else if(len < 0) {
            if(errno == EAGAIN || errno == EINTR) {
                continue;
            } else {
                break;
            }
        }
        fullname += std::string(buf, len);
    }

    close(p_stdout[0]);

    pid_t wpid;
    int status;

    do {
        wpid = waitpid(pid, &status, 0);
    } while(wpid == -1 && errno == EINTR);

    if(wpid == -1 || !(WIFEXITED(status)) ||
            (WEXITSTATUS(status) != 0)) {
        return std::string();
    }

    if(!fullname.empty() && fullname[fullname.size()-1] == '\n')
        fullname.resize(fullname.size()-1);

    if(!fullname.empty() && !dir.empty()) {
#ifndef WINDOWS
        if(fullname[0] != PATH_SEP)
#else
        if(fullname.size() < 3 ||
                (fullname[1] == ':' && fullname[2] == PATH_SEP))
#endif
        {
            fullname = dir + PATH_SEP + fullname;
        }
    }

    return fullname;
}

} // namespace texpp

