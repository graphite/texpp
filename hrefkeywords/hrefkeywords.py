#!/usr/bin/env python

import sys
import texpy

import latexstubs

def loadConcepts(conceptsfile):
    """ Loads concepts from the files
        and arranges them in a dictionary """
    concepts = {}
    for line in conceptsfile:
        line = line.strip(' \n\r')
        line1 = line.lower() \
                    .replace('\\-', '-') \
                    .replace('\\(', '(') \
                    .replace('\\)', ')')

        word = ''
        item = concepts
        for c in line1:
            if c.isalpha():
                word += c
            else:
                if word:
                    item = item.setdefault(word, {})
                    word = ''
                if not c.isspace():
                    item = item.setdefault(c, {})

        if word:
            item = item.setdefault(word, {})

        # a special item that marks the end of the sequence
        item[True] = line1

    return concepts

def parseDocument(filename, fileobj):
    """ Parses the document using TeXpp """
    parser = texpy.Parser(filename, fileobj, '', False, True)

    # Mimic the most important parts of LaTeX style
    latexstubs.initLaTeXstyle(parser)

    # Do the real work
    return parser.parse()

def doReplace(node, macro, concepts, filename):
    """ Recursively replaces concepts in the node and returns
        a pair (modified source, replacement stats) """

    childrenCount = node.childrenCount()

    # Do nothing for leaf nodes
    if childrenCount == 0:
        return (node.source(filename), {})

    src = ''
    n = 0
    stats = {}
    while n < childrenCount:
        child = node.child(n)

        # Try replacing text_word or text_character
        if child.type() in ('text_word', 'text_character'):
            item = concepts
            max_m = None

            for m in xrange(n, childrenCount):
                childm = node.child(m)

                # look in concepts
                if childm.type() in ('text_word', 'text_character'):
                    item = item.get(childm.value().lower(), None)

                    if item is None: # not found
                        break
                    elif True in item: # do found
                        max_m = m # but keep looking for bigger one
                        word = item[True]
                        
                # skip spaces, break on anything else
                elif childm.type() != 'text_space':
                    break

            if max_m is not None:
                # do replace
                src += '\\' + macro + \
                    '{http://en.wikipedia.org/wiki/' + word + '}{'
                        
                for m in xrange(n, max_m+1):
                    src += node.child(m).value()

                src += '}'
                n = max_m
                stats.setdefault(word, 0)
                stats[word] += 1

            else:
                # nothing found
                src += child.source(filename)

        # Walk recursively if whitelisted
        elif child.type() in latexstubs.whitelistEnvironments:
            (src1, stats1) = doReplace(child, macro, concepts, filename)
            src += src1
            for w,c in stats1.iteritems():
                stats.setdefault(w, 0)
                stats[w] += c

        # Just grab the source otherwise
        else:
            src += child.source(filename)

        n += 1

    return (src, stats)

def main():
    """ Main routine """
    # Define command line options
    from optparse import OptionParser
    optparser = OptionParser(usage='%prog [options] texfile')
    optparser.add_option('-c', '--concepts', type='string', help='concepts file')
    optparser.add_option('-o', '--output', type='string', help='output file')
    optparser.add_option('-m', '--macro', type='string', default='href', 
                                    help='macro (default: href)')
    optparser.add_option('-s', '--stats', action='store_true', help='print stats')

    # Parse command line options
    opt, args = optparser.parse_args()

    # Additional options verification
    if opt.concepts is None:
        optparser.error('Required option --concepts was not specified')

    if len(args) != 1:
        optparser.error('Wrong command line arguments')

    # Open input file
    try:
        filename = args[0]
        fileobj = open(filename, 'r')
    except IOError:
        optparser.error('Can not open input file (\'%s\')' % (filename,))

    # Open concepts file
    try:
        conceptsfile = open(opt.concepts, 'r')
    except IOError:
        optparser.error('Can not open concepts file (\'%s\')' % (opt.concepts,))

    # Open output file
    if opt.output:
        try:
            outfile = open(opt.output, 'w')
        except IOError:
            optparser.error('Can not open output file (\'%s\')' % (opt.output,))
    else:
        outfile = sys.stdout

    # Load concepts
    concepts = loadConcepts(conceptsfile)

    # Load and parse the document
    document = parseDocument(filename, fileobj)

    # Do the main job
    (new_source, stats) = doReplace(document,
                opt.macro, concepts, filename)

    # Output
    outfile.write(new_source)
    outfile.flush()

    # Stats
    if opt.stats:
        for w,n in stats.iteritems():
            print 'KEYWORD <%s> replaced %d times' % (w, n)

if __name__ == '__main__':
    main()

