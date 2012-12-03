#!/usr/bin/python

import os
import sys
import shutil
import urllib2
import random
import optparse
import gzip
import tarfile
import StringIO
import re

THISDIR = os.path.dirname(__file__)

TESTHREF_DEF = r"""
\def\testhref#1#2{\immediate\write16{REPLACE}\relax#2}
"""

class CommandError(Exception):
    pass

class TestError(Exception):
    ET_SUCCESS = 0
    ET_TEST_ERROR = 1
    ET_WARNING = 2
    ET_FAIL = 3

    def __init__(self, message, etype):
        super(TestError, self).__init__(message)
        self.etype = etype

    def result(self):
        if self.etype == self.ET_SUCCESS:
            return 'success'
        elif self.etype == self.ET_TEST_ERROR:
            return 'test_error'
        elif self.etype == self.ET_WARNING:
            return 'warning'
        elif self.etype == self.ET_FAIL:
            return 'fail'

def unpack_article_from_arxiv(filename, outdir, fileobj=None):
    if os.path.exists(outdir):
        shutil.rmtree(outdir, True)

    os.mkdir(outdir)

    if fileobj is None:
        try:
            fileobj = file(filename, 'r')
        except IOError:
            return False

    try:
        gzipobj = gzip.GzipFile(fileobj=fileobj)
    except IOError:
        return False 
        
    try:
        tarobj = tarfile.TarFile(fileobj=gzipobj, name=filename)
        # Mutliple files
        for name in tarobj.getnames():
            nname = os.path.normpath(name)
            if os.path.isabs(nname) or nname.startswith('..'):
                return False
        tarobj.extractall(outdir)

    except (tarfile.TarError, IOError):
        # Single file
        try:
            gzipobj.seek(0)
            outfile = file(os.path.join(outdir, 'main.tex'), 'w')
            outfile.write(gzipobj.read())
            outfile.close()
            gzipobj.close()
        except IOError:
            return False

    return True

def find_main_latex_file(dirname):
    for fname in os.listdir(dirname):
        # Only consider files with .tex extensions
        if fname.endswith('.tex'):
            fnamefull = os.path.join(dirname, fname)
            # Look for \documentclass in file
            fobj = file(fnamefull, 'r')
            for line in fobj:
                if line.find('\\documentclass')>=0 or \
                        line.find('\\documentstyle')>=0:
                    fobj.close()
                    return fname
            fobj.close()
    return None

def fix_latex_hyperref(filename, testhref=False):
    # Create backup file
    shutil.copy(filename, filename + '.bak')
    fobj = file(filename + '.bak', 'r')
    dobj = file(filename, 'w')

    if testhref:
        data = fobj.read()
        dobj.write(TESTHREF_DEF)
        dobj.write(data)
        return True

    # Check wether hyperref is already included
    hyperref_found = False
    hyperref_re = re.compile(
        r'^([^%]*\\usepackage\s*)(?:\[([^]]*)\])?(\s*{\s*hyperref\s*})')
    end_re = re.compile(r'^[^%]*\\end\s*{\s*document\s*}')
    for line in fobj:
        m = hyperref_re.match(line)
        if m:
            if m.group(2):
                if m.group(2).find('breaklinks') < 0:
                    line = hyperref_re.sub(r'\1[breaklinks,\2]\3', line)
            else:
                line = hyperref_re.sub(r'\1[breaklinks]\3', line)
            hyperref_found = True

        if not hyperref_found and end_re.match(line):
            break

        dobj.write(line)

    if not hyperref_found:
        # We have to insert hyperref ourselfs
        fobj.seek(0)
        dobj.seek(0)
        dobj.truncate()
        documentclass_re = re.compile(
            r'^[^%]*\\documentclass\s*([^]]*])?\s*{\s*[^}]*\s*}')
        for line in fobj:
            dobj.write(line)
            if documentclass_re.match(line):
                dobj.write('\\usepackage[breaklinks]{hyperref}\n')
                hyperref_found = True

    fobj.close()
    dobj.close()

    if not hyperref_found:
        raise CommandError('Can not automatically insert hyperref package')

    return True

def compile_latex(filename, verbose=False):
    oldcwd = os.getcwd()
    os.chdir(os.path.dirname(filename))

    basename = os.path.basename(filename)
    jobname = os.path.splitext(basename)[0]

    # XXX: do we need to rerun latex multiple times ?
    if verbose:
        latex_cmd = 'latex -interaction=nonstopmode "%s"' % (basename,)
    else:
        latex_cmd = 'latex -interaction=batchmode "%s" > /dev/null' % (
                                                             basename,)
    ret = os.system(latex_cmd)
        
    os.chdir(oldcwd)
    if ret:
        raise CommandError('latex error: %d' % (ret,))

def compare_files(origfile, replfile, ftype, skip_bytes=0):
    # Open files
    try:
        origobj = file(origfile, 'r')
    except IOError,e:
        raise TestError('Can not open original %s file for %s: %s' % (
            ftype, origfile, e), TestError.ET_TEST_ERROR)
    try:
        replobj = file(replfile, 'r')
    except IOError,e:
        raise TestError('Can not open modified %s file for %s: %s' % (
            ftype, origfile, e), TestError.ET_FAIL)

    # Skip first bytes
    origobj.read(skip_bytes)
    replobj.read(skip_bytes)

    # Compile files
    while 1:
        odata = origobj.read(1024)
        rdata = replobj.read(1024)
        if odata != rdata:
            return False
        if not odata:
            break

    origobj.close()
    replobj.close()

    return True

def test_one_file(fname, opt):
    # Prepare the work dir
    wdir = os.path.join(opt.work_dir, fname)
    if os.path.exists(wdir):
        shutil.rmtree(wdir, True)

    os.mkdir(wdir)

    # Unpack the file
    unpackdir = os.path.join(wdir, 'unpack')
    if not unpack_article_from_arxiv(
                os.path.join(opt.tex_dir, fname), unpackdir):
        raise TestError('Can not unpack the article',
                                    TestError.ET_TEST_ERROR)

    # Find main tex file
    texfile = find_main_latex_file(unpackdir)
    if not texfile:
        raise TestError('Can not find main TeX file in the archive',
                                    TestError.ET_TEST_ERROR)

    # Prepare dirs for test
    origdir = os.path.join(wdir, 'orig')
    repldir = os.path.join(wdir, 'repl')
    origfile = os.path.join(origdir, texfile)
    replfile = os.path.join(repldir, texfile)

    shutil.copytree(unpackdir, origdir)
    shutil.copytree(unpackdir, repldir)

    # Add hyperref package or fix its options
    try:
        fix_latex_hyperref(origfile, not opt.real_href)
    except CommandError, e:
        raise TestError('Can not automatically insert hyperref package',
                                    TestError.ET_TEST_ERROR)

    # Prepare replacer command
    if opt.real_href:
        macro = 'href'
    else:
        macro = 'testhref'

    replacer_cmd = opt.cmd % { 'macro': macro,
                               'source': '"' + origfile + '"',
                               'output': '"' + replfile + '"' }
    os.system('echo "***" %s >> %s' % (replacer_cmd, opt.cmd_log_file))

    if not opt.verbose:
        replacer_cmd += ' 2>&1 >> ' + opt.cmd_log_file
    else:
        replacer_cmd += ' 2>&1 | tee -a ' + opt.cmd_log_file

    # Run replacer
    ret = os.system(replacer_cmd)
    if ret:
        raise TestError('Error running test program',
                                    TestError.ET_FAIL)

    # Compile original
    try:
        compile_latex(origfile, opt.verbose)
    except CommandError, e:
        raise TestError('Can not compile original document %s: %s' % (
            origfile, e), TestError.ET_TEST_ERROR)

    # Check whether the file was actually changed
    if compare_files(origfile, replfile, 'tex'):
        raise TestError('No replacements was made', TestError.ET_WARNING)

    # Compile replaced
    try:
        compile_latex(replfile, opt.verbose)
    except CommandError, e:
        raise TestError('Can not compile modified document %s: %s' % (
            replfile, e), TestError.ET_FAIL)

    # Compare DVIs only in testhref mode
    if not opt.real_href:
        if not compare_files(os.path.splitext(origfile)[0] + '.dvi',
                      os.path.splitext(replfile)[0] + '.dvi',
                      'dvi', 100):
            raise TestError('Generated DVI files differ',
                                TestError.ET_WARNING)

    return 'success'

def main(argv):
    optparser = optparse.OptionParser()

    cmd_default = './hrefkeywords.py -c ./concepts4.txt ' + \
                '-s -m %(macro)s -o %(output)s %(source)s'

    optparser.add_option('-t', '--tex-dir', default='tex',
                help='Directory with original articles from arxiv.org')
    optparser.add_option('-f', '--tex-file', action='append',
                help='Select test files to process ' + \
                     '(can be specified multiple times, ' + \
                     'default: all files in TEX_DIR)')
    optparser.add_option('-w', '--work-dir', default='work',
                help='Directory for working files')
    optparser.add_option('-l', '--log-file', default='test.log',
                help='File to log test results')
    optparser.add_option('-L', '--cmd-log-file', default='test_cmd.log',
                help='Log file for command output')
    optparser.add_option('-m', '--macros-dir', default='texmacros',
                help='Folder with extra TeX macros from arxiv.org')
    optparser.add_option('-r', '--real-href', action='store_true',
                help='Use real href instead of a fake macro')
    optparser.add_option('-v', '--verbose', action='store_true',
                help='Show program output')
    optparser.add_option('-c', '--cmd', default=cmd_default,
                help='Replacer command (default=%s)' % (cmd_default,))

    opt, args = optparser.parse_args(argv)

    os.environ['TEXINPUTS'] = os.environ.get('TEXINPUTS', '.:') + ':' + \
                                os.path.abspath(opt.macros_dir)

    if not os.path.exists(opt.work_dir):
        os.mkdir(opt.work_dir)

    if opt.cmd_log_file:
        try:
            os.remove(opt.cmd_log_file)
        except OSError:
            pass

    testcount = 0
    testerror = 0
    testfail = 0
    testwarning = 0
    logfile = file(opt.log_file, 'w')

    if opt.tex_file:
        files = opt.tex_file
    else:
        files = os.listdir(opt.tex_dir)

    filescount = len(files)
    filenum = 0

    for fname in files:
        filenum += 1

        sys.stdout.write('[%d of %d] %s:' % (
                        filenum, filescount, fname,))
        sys.stdout.flush()
        if opt.verbose:
            sys.stdout.write('\n')

        testcount += 1
        logfile.write('%s: ' % (fname,))
        logfile.flush()

        try:
            result = test_one_file(fname, opt)
            etype = TestError.ET_SUCCESS
        except TestError, e:
            result = e.result() + ': ' + str(e)
            etype = e.etype

        logfile.write(result + '\n')
        logfile.flush()

        if opt.verbose:
            sys.stdout.write(' ...')

        sys.stdout.write(' %s\n' % (result,))
    
        if etype == TestError.ET_TEST_ERROR:
            testerror += 1
        elif etype == TestError.ET_WARNING:
            testwarning += 1
        elif etype == TestError.ET_FAIL:
            testfail += 1

    msg = '%d (of %d) tests failed, %d warnings, %d tests broken' % (
            testfail, testcount - testerror, testwarning, testerror)
    if logfile:
        logfile.write(msg + '\n')

    sys.stdout.write(msg + '\n')

    sys.exit(testfail)

if __name__ == '__main__':
    main(sys.argv)

