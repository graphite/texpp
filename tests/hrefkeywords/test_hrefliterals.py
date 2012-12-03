#!/usr/bin/env python

import sys
sys.path.insert(0, sys.argv[1])
sys.path.insert(0, sys.argv[2])
sys.path.insert(0, sys.argv[3])
sys.argv = sys.argv[0:1] + sys.argv[4:]

import hrefliterals
import StringIO
import unittest
import os

class NormLiteralTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(NormLiteralTest, self).__init__(*args, **kwargs)
        self.stemmer = hrefliterals.Stemmer()
        self.words = hrefliterals.WordsDict(
                    '/usr/share/dict/words', 4)
        self.words.insert('I')
        self.words.insert('a')

    def normLiteral(self, literal):
        return hrefliterals.normLiteral(
                literal, self.words, self.stemmer)

    def testLongWords(self):
        self.assertEqual(self.normLiteral('electrons'), 'electron')
        self.assertEqual(self.normLiteral('xxxxxxxxx'), 'xxxxxxxxx')
        self.assertEqual(self.normLiteral('eLectrons'), 'E.L.E.C.T.R.O.N.S.')

    def testShortWords(self):
        self.assertEqual(self.normLiteral('or'), 'or')
        self.assertEqual(self.normLiteral('set'), 'set')
        self.assertEqual(self.normLiteral('sets'), 'set')
        self.assertEqual(self.normLiteral('SET'), 'S.E.T.')
        self.assertEqual(self.normLiteral('S.E.T.'), 'S.E.T.')
        self.assertEqual(self.normLiteral('s.e.t.'), 'S.E.T.')
        self.assertEqual(self.normLiteral('seT'), 'S.E.T.')

    def testShortNonWords(self):
        self.assertEqual(self.normLiteral('dof'), 'D.O.F.')
        self.assertEqual(self.normLiteral('Dof'), 'D.O.F.')
        self.assertEqual(self.normLiteral('DoF'), 'D.O.F.')

    def testShortNames(self):
        self.assertEqual(self.normLiteral('Ada'), 'ada')
        self.assertEqual(self.normLiteral('ada'), 'A.D.A.')

    def testOneLetter(self):
        self.assertEqual(self.normLiteral('I'), 'i')
        self.assertEqual(self.normLiteral('i'), 'I.')
        self.assertEqual(self.normLiteral('A'), 'a')
        self.assertEqual(self.normLiteral('a'), 'a')
        self.assertEqual(self.normLiteral('T'), 'T.')
        self.assertEqual(self.normLiteral('t'), 'T.')

    def testSAbbr(self):
        self.assertEqual(self.normLiteral('Dr.A.B.'), 'DR.A.B.')
        self.assertEqual(self.normLiteral('Dr.A.Boy.'), 'DR.A.BOY.')
        self.assertEqual(self.normLiteral('Dr.A.Boy'), 'DR.A.boi')
        self.assertEqual(self.normLiteral('Dr.A.boy'), 'DR.A.boi')
        self.assertEqual(self.normLiteral('Dr.A.Ada'), 'DR.A.ada')
        self.assertEqual(self.normLiteral('Dr.A.ada'), 'DR.A.A.D.A.')
        self.assertEqual(self.normLiteral('Dr.A.B.'), 'DR.A.B.')

    def testDigits(self):
        self.assertEqual(self.normLiteral('Fig.1'), 'FIG.1.')
        self.assertEqual(self.normLiteral('SU(2)'), 'S.U.(2.)')

    def testIgnoredSymbols(self):
        self.assertEqual(self.normLiteral('g-factor'), 'G.factor')
        self.assertEqual(self.normLiteral('g/factor'), 'G.factor')
        self.assertEqual(self.normLiteral('g.factor'), 'G.factor')
        self.assertEqual(self.normLiteral('g factor'), 'G.factor')
        self.assertEqual(self.normLiteral('T-J model'), 'T.J.model')

    def testPluralAbbr(self):
        self.assertEqual(self.normLiteral('SETs'), 'S.E.T.')
        self.assertEqual(self.normLiteral('SETes'), 'S.E.T.')
        self.assertEqual(self.normLiteral('SETS'), 'S.E.T.S.')
        self.assertEqual(self.normLiteral('SETES'), 'S.E.T.E.S.')
        self.assertEqual(self.normLiteral('S.E.T.s'), 'S.E.T.S.')
        self.assertEqual(self.normLiteral('dofs'), 'D.O.F.S.')

    def testApostrophe(self):
        self.assertEqual(self.normLiteral('world\'s'), 'world')
        self.assertEqual(self.normLiteral('\'s x'), '\'S.X.')
        self.assertEqual(self.normLiteral(' \'s x'), '\'S.X.')

    def testArticle(self):
        self.assertEqual(self.normLiteral('The word'), 'word')
        self.assertEqual(self.normLiteral('a word'), 'word')
        self.assertEqual(self.normLiteral('an apple'), 'appl')
        self.assertEqual(self.normLiteral('2 the word'), '2.word')
        self.assertEqual(self.normLiteral('word a'), 'worda')

    def testMultiWords(self):
        self.assertEqual(self.normLiteral('hello world'), 'helloworld')
        self.assertEqual(self.normLiteral('hello-world'), 'helloworld')
        self.assertEqual(self.normLiteral('hello/world'), 'helloworld')
        self.assertEqual(self.normLiteral('hello+world'), 'hello+world')
        self.assertEqual(self.normLiteral('test SET'), 'testS.E.T.')

class LiteralFunctionsTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(LiteralFunctionsTest, self).__init__(*args, **kwargs)
        self.exclude_re = '.*equation.*|.*eqn.*'
        self.stemmer = hrefliterals.Stemmer()
        self.words = hrefliterals.WordsDict(
                    '/usr/share/dict/words', 4)
        self.source = r"""
            \begin{document}words 1 word2 wo%
                       rd
                \begin{equation}word\end{equation}
            \end{document}
        """
        self.document = hrefliterals.parseDocument(
                        'f', StringIO.StringIO(self.source), os.getcwd())

    def testExtractTextInfo(self):
        textTags = hrefliterals.extractTextInfo(
                self.document, self.exclude_re, '')
        self.assertEqual(textTags.keys(), ['f'])
        self.assertEqual(len(textTags['f']), 11)
        self.assertEqual(textTags['f'][0], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.WORD, 29, 34, 'words'))
        self.assertEqual(textTags['f'][1], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.CHARACTER, 34, 35, ' '))
        self.assertEqual(textTags['f'][2], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.CHARACTER, 35, 36, '1'))
        self.assertEqual(textTags['f'][3], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.CHARACTER, 36, 37, ' '))
        self.assertEqual(textTags['f'][4], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.WORD, 37, 41, 'word'))
        self.assertEqual(textTags['f'][5], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.CHARACTER, 41, 42, '2'))
        self.assertEqual(textTags['f'][6], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.CHARACTER, 42, 43, ' '))
        self.assertEqual(textTags['f'][7], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.WORD, 43, 72, 'word'))
        self.assertEqual(textTags['f'][8], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.CHARACTER, 72, 73, ' '))
        self.assertEqual(textTags['f'][9], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.CHARACTER, 123, 124, ' '))
        self.assertEqual(textTags['f'][10], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.CHARACTER, 150, 151, ' '))

    def testFindLiterals(self):
        textTags = hrefliterals.extractTextInfo(
                self.document, self.exclude_re, '')
        literals = {'word':None, 'W.O.R.D.2.':None}
        literalTags = hrefliterals.findLiterals(
                textTags['f'], literals, {}, self.words, self.stemmer, 0)
        self.assertEqual(len(literalTags), 3)
        self.assertEqual(literalTags[0], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.LITERAL, 29, 34, 'word'))
        self.assertEqual(literalTags[1], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.LITERAL, 37, 42, 'W.O.R.D.2.'))
        self.assertEqual(literalTags[2], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.LITERAL, 43, 72, 'word'))

    def testReplaceTags(self):
        textTags = hrefliterals.extractTextInfo(
                self.document, self.exclude_re, '')
        literals = {'word':None, 'W.O.R.D.2.':None}
        literalTags = hrefliterals.findLiterals(
                textTags['f'], literals, {}, self.words, self.stemmer, 0)
        literalTags[0].value = '*'
        literalTags[1].value = '**'
        literalTags[2].value = '***'
        replaced = hrefliterals.replaceLiterals(self.source, literalTags)
        rsource = r"""
            \begin{document}* 1 ** ***
                \begin{equation}word\end{equation}
            \end{document}
        """
        self.assertEqual(replaced, rsource)

class LiteralsTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(LiteralsTest, self).__init__(*args, **kwargs)
        self.exclude_re = '.*equation.*|.*eqn.*'
        self.stemmer = hrefliterals.Stemmer()
        self.words = hrefliterals.WordsDict(
                    '/usr/share/dict/words', 4)

    def findLiterals(self, source, literals, notLiterals):
        document = hrefliterals.parseDocument('f', StringIO.StringIO(source),
                                                os.getcwd())
        textTags = hrefliterals.extractTextInfo(document, self.exclude_re, '')
        return hrefliterals.findLiterals(textTags['f'], literals, notLiterals,
                                                self.words, self.stemmer, 0)

    def testAdjacentChars(self):
        literalTags = self.findLiterals(' spin -spin spin- spin-1 spin-12',
                           dict.fromkeys(('spin', 'spin1.')), {})
        self.assertEqual(len(literalTags), 2)
        self.assertEqual(literalTags[0], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.LITERAL, 0, 5, 'spin'))
        self.assertEqual(literalTags[1], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.LITERAL, 18, 24, 'spin1.'))

    def testNonLiterals(self):
        literalTags = self.findLiterals(' DE de i.e. ',
                            dict.fromkeys(('D.E.', 'I.E.')), {})
        self.assertEqual(len(literalTags), 3)
        literalTags = self.findLiterals(' DE de i.e. ',
                            dict.fromkeys(('D.E.', 'I.E.')),
                            dict.fromkeys(('de', 'i.e.')))
        self.assertEqual(len(literalTags), 1)
        self.assertEqual(literalTags[0], hrefliterals.TextTag(
                    hrefliterals.TextTag.Type.LITERAL, 0, 3, 'D.E.'))

if __name__ == '__main__':
    unittest.main()

