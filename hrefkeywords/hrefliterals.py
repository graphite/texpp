#!/usr/bin/env python

import sys
import texpy

import latexstubs

import codecs
import re
import os

from _chrefliterals import \
    Stemmer, WordsDict, TextTag, TextTagList, \
    absolutePath, isLocalFile, normLiteral, extractTextInfo, \
    getDocumentEncoding, findLiterals, replaceLiterals

ABBR_MAX = 4

excludedEnvironments = \
    r'.*math.*|.*equation.*|.*eqn.*|.*array.*|' + \
    r'.*align.*|.*multiline.*|.*gather.*|' + \
    r'.*verbatim.*|.*biblio.*'

knownNotLiterals = dict.fromkeys((
    'i.e.', 'ie.',
    'c.f.', 'cf.', 'cf',
    'e.g.', 'eg.',
    'de', 'De' # for de in the names of Universities
))

def loadLiteralsFromConcepts4(conceptsfile, words, stemmer):
    """ Loads concepts from the files
        and arranges them in a dictionary """
    literals = {}
    for line in conceptsfile:
        line = line.strip(' \n\r')
        line1 = line.lower() \
                    .replace('\\-', '-') \
                    .replace('\\(', '(') \
                    .replace('\\)', ')')

        literals.setdefault(
                normLiteral(line1, words, stemmer), []) \
            .append(line1)

    return literals

def parseDocument(filename, fileobj, workdir):
    """ Parses the document using TeXpp """
    parser = texpy.Parser(filename, fileobj, workdir, False, True)

    # Mimic the most important parts of LaTeX style
    latexstubs.initLaTeXstyle(parser)

    # Do the real work
    return parser.parse()

def main():
    """ Main routine """

    # Define command line options
    from optparse import OptionParser
    optparser = OptionParser(usage='%prog [options] texfile')
    optparser.add_option('-c', '--concepts', type='string', help='concepts file')
    optparser.add_option('-o', '--output', type='string', help='output directory')
    optparser.add_option('-m', '--macro', type='string', default='href', 
                                    help='macro (default: href)')
    optparser.add_option('-s', '--stats', action='store_true', help='print stats')
    optparser.add_option('-t', '--timings', action='store_true', help='print timings')

    # Parse command line options
    opt, args = optparser.parse_args()

    # Additional options verification
    if opt.concepts is None:
        optparser.error('Required option --concepts was not specified')

    if opt.output is None:
        optparser.error('Required option --output was not specified')

    if len(args) != 1:
        optparser.error('Wrong command line arguments')

    # Open input file
    try:
        filename = os.path.abspath(args[0])
        workdir, rest = os.path.split(filename)
        fileobj = open(filename, 'r')
    except IOError, e:
        optparser.error('Can not open input file (\'%s\'): %s' % \
                                (filename, str(e)))

    # Open concepts file
    try:
        conceptsfile = open(opt.concepts, 'r')
    except IOError, e:
        optparser.error('Can not open concepts file (\'%s\'): %s' % \
                                (opt.concepts, str(e)))

    # Check output dir
    if not os.path.exists(opt.output):
        try:
            os.mkdir(opt.output)
        except (IOError, OSError), e:
            optparser.error('Can not create output directory (\'%s\'): %s' % \
                                (opt.output, str(e)))

    if not os.path.isdir(opt.output):
        optparser.error('\'%s\' is not a directory')

    # Load words and create stemmer
    stemmer = Stemmer()
    words = WordsDict('/usr/share/dict/words', ABBR_MAX)
    words.insert('I')
    words.insert('a')

    # Load concepts
    literals = loadLiteralsFromConcepts4(conceptsfile, words, stemmer)
    conceptsfile.close()

    import time
    timings = []

    # Load and parse the document
    tm = time.time()
    document = parseDocument(filename, fileobj, workdir)
    timings.append('parseDocument: %f' % (time.time()-tm,))
    fileobj.close()

    # Extract text tags
    tm = time.time()
    textTags = extractTextInfo(document, excludedEnvironments, workdir)
    timings.append('extractTextInfo: %f' % (time.time()-tm,))

    # Find literals in each file
    replaced = {}
    foundLiterals = {}
    for f, tags in textTags.iteritems():
        assert f is not None and isLocalFile(f, workdir)
        tm = time.time()
        literalTags = findLiterals(tags, literals, knownNotLiterals,
                                            words, stemmer, 0)
        timings.append('findLiterals: %f' % (time.time()-tm,))
        tm = time.time()
        source = open(os.path.join(workdir, f), 'r').read()
        timings.append('readSourceFile: %f' % (time.time()-tm,))
        tm = time.time()
        for t in literalTags:
            foundLiterals[t.value] = foundLiterals.get(t.value, 0) + 1
            t.value = ''.join(('\\href{', t.value, '}{',
                            source[t.start:t.end], '}'))
        timings.append('prepareReplacements: %f' % (time.time()-tm,))
        tm = time.time()
        replaced[f] = replaceLiterals(source, literalTags)
        timings.append('replaceLiterals: %f' % (time.time()-tm,))

    # Save results
    for f, s in replaced.iteritems():
        assert f is not None and isLocalFile(f, workdir)
        fname = os.path.join(opt.output, f)
        try:
            outfile = open(fname, 'w')
        except IOError, e:
            sys.stdout('Can not open output file (\'%s\'): %s' % \
                                (fname, str(e)))
            continue
        tm = time.time()
        outfile.write(s)
        outfile.close()
        timings.append('writeSourceFile: %f' % (time.time()-tm,))

    # Stats
    if opt.stats:
        for w,n in foundLiterals.iteritems():
            print 'Concept <%s> replaced %f times' % (w, n)

    if opt.timings:
        for l in timings:
            print l

if __name__ == '__main__':
    main()

