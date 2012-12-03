#!/usr/bin/env python

import sys
import texpy

def get_text(node, whitelist):
    text = ''

    for name, child in node.children():
        if child.type() in ('text_word', 'text_character', 'text_space'):
            text += child.value()
        elif not whitelist or child.type() in whitelist:
            text += get_text(child, whitelist)

    return text

def main():
    """ Main routine """
    # Define command line options
    from optparse import OptionParser
    optparser = OptionParser(usage='%prog [options] texfile')
    optparser.add_option('-t', '--print-tree', action='store_true',
                                    help='print document tree before exit')
    optparser.add_option('-x', '--print-text', action='store_true',
                                    help='print document text before exit')
    optparser.add_option('-s', '--use-stubs', action='store_true',
                                    help='load stubs from latexstubs.py')
    optparser.add_option('-q', '--queit', action='store_true',
                                    help='hide parser log messaged')

    # Parse command line options
    opt, args = optparser.parse_args()

    if len(args) == 0:
        # Input from terminal
        interactive = True
        filename = ''
        fileobj = None
    elif len(args) == 1:
        # Open input file
        interactive = False
        filename = args[0]
        try:
            filename = args[0]
            fileobj = open(filename, 'r')
        except IOError:
            optparser.error('Can not open input file (\'%s\')' % (filename,))
    else:
        optparser.error('Wrong command line arguments')
        
    # Create logger
    if opt.queit:
        logger = texpy.NullLogger()
    else:
        logger = texpy.ConsoleLogger()

    # Create the parser
    parser = texpy.Parser(filename, fileobj, '', interactive, False, logger)

    # Load latexstubs
    if opt.use_stubs:
        # Mimic the most important parts of LaTeX style
        import latexstubs
        latexstubs.initLaTeXstyle(parser)
        whitelist = latexstubs.whitelistEnvironments
    else:
        whitelist = None

    doc = parser.parse()
    
    # Print tree
    if opt.print_tree:
        print doc.treeRepr()

    # Print text
    if opt.print_text:
        print get_text(doc, whitelist)

if __name__ == '__main__':
    main()

