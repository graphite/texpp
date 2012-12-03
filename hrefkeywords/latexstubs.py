import texpy

class BeginCommand(texpy.Command):
    def invoke(self, parser, node):
        group = parser.parseGroup(texpy.Parser.GroupType.NORMAL)
        node.appendChild('env_type', group)

        env_type = ''
        for n,c in group.children():
            if n == 'text_word':
                env_type = c.value()
                break

        parser.beginCustomGroup('environment_' + env_type)
        return True

class EndCommand(texpy.Command):
    def invoke(self, parser, node):
        group = parser.parseGroup(texpy.Parser.GroupType.NORMAL)
        node.appendChild('env_type', group)

        env_type = ''
        for n,c in group.children():
            if n == 'text_word':
                env_type = c.value()
                break

        if env_type == 'end':
            parser.end()

        parser.endCustomGroup()
        return True

def parseOptionalArgs(parser):
    node = texpy.Node('optional_args')
    node.appendChild('optional_spaces', parser.parseOptionalSpaces())
    
    if parser.peekToken() and parser.peekToken().isCharacter('['):
        args = texpy.Node('args')
        node.appendChild('args', args)
        value = ''
        while parser.peekToken():
            value += parser.nextToken(args.tokens()).value
            if parser.lastToken().isCharacter(']'):
                break
        node.setValue(value[1:-1])

    return node

def parseGeneralArg(parser, expand=False):
    node = texpy.Node('args')
    node.appendChild('optional_spaces', parser.parseOptionalSpaces())

    value = ''
    if parser.peekToken() and parser.peekToken().isCharacter('{'):
        textNode = parser.parseGeneralText(expand)
        node.appendChild('group', textNode)
        for t in textNode.child('balanced_text').tokens():
            value += t.value
    else:
        tNode = parser.parseToken()
        node.appendChild('token', tNode)
        value = '' # XXX

    node.setValue(value)
    return node

class Usepackage(texpy.Command):
    def invoke(self, parser, node):
        argNode = parseOptionalArgs(parser)
        pkgNode = parseGeneralArg(parser)
        node.appendChild('args', argNode)
        node.appendChild('package', pkgNode)

        if pkgNode.value() == 'inputenc':
            node.setType('inputenc')
            node.setValue(argNode.value())

        return True

class Newcommand(texpy.Command):
    def invoke(self, parser, node):
        node.appendChild('cmd', parseGeneralArg(parser))
        node.appendChild('args', parseOptionalArgs(parser))
        node.appendChild('opt', parseOptionalArgs(parser))
        node.appendChild('def', parseGeneralArg(parser))

        return True

class Newenvironment(texpy.Command):
    def invoke(self, parser, node):
        node.appendChild('nam', parseGeneralArg(parser))
        node.appendChild('args', parseOptionalArgs(parser))
        node.appendChild('begdef', parseGeneralArg(parser))
        node.appendChild('enddef', parseGeneralArg(parser)) 

        return True

class Newtheorem(texpy.Command):
    def invoke(self, parser, node):
        if parser.peekToken().isCharacter('*'):
            node.appendChild('star', parser.parseToken())

        node.appendChild('env_nam', parseGeneralArg(parser))

        node.appendChild('optional_spaces', parser.parseOptionalSpaces())
        if parser.peekToken() and parser.peekToken().isCharacter('['):
            node.appendChild('numbered_like', parseOptionalArgs(parser))
            node.appendChild('caption', parseGeneralArg(parser))
        else:
            node.appendChild('caption', parseGeneralArg(parser))
            node.appendChild('within', parseOptionalArgs(parser))

        return True

class Documentclass(texpy.Command):
    def invoke(self, parser, node):
        node.appendChild('options', parseOptionalArgs(parser))
        node.appendChild('class', parseGeneralArg(parser))

        return True

class DefCommand(texpy.Command):
    def invoke(self, parser, node):
        node.appendChild('token', parser.parseControlSequence())
        args = texpy.Node('def_args')
        node.appendChild('args', args)
        while parser.peekToken() and \
                not parser.peekToken().isCharacterCat(
                            texpy.Token.CatCode.BGROUP):
            parser.nextToken(args.tokens())
        
        node.appendChild('def', parser.parseGeneralText(False))

        return True

    def checkPrefixes(self, parser):
        return True

class InputCommand(texpy.Command):
    def invoke(self, parser, node):
        fnameNode = parser.parseFileName()
        node.appendChild('file_name', fnameNode)

        fname = fnameNode.value() or ''
        if fname.startswith('{'):
            fname = fname[1:-1]

        # Temporary disable this package
        if fname == 'xy':
            return True

        fullname = texpy.kpsewhich(fname, parser.workdir())

        parser.input(fname, fullname)
        return True

def createCommand(parser, name, cmd, *args):
    parser.setSymbol('\\' + name, cmd('\\' + name, *args))

def initLaTeXstyle(parser):
    """ Defines the most important aspects of LaTeX style """

    parser.setSymbol('catcode'+str(ord('{')),
                int(texpy.Token.CatCode.BGROUP))
    parser.setSymbol('catcode'+str(ord('}')),
                int(texpy.Token.CatCode.EGROUP))
    parser.setSymbol('catcode'+str(ord('$')),
                int(texpy.Token.CatCode.MATHSHIFT))
    parser.setSymbol('catcode'+str(ord('\t')),
                int(texpy.Token.CatCode.SPACE))

    createCommand(parser, 'begin', BeginCommand)
    createCommand(parser, 'end', EndCommand)

    createCommand(parser, 'usepackage', Usepackage)

    createCommand(parser, 'newcommand', Newcommand)
    createCommand(parser, 'renewcommand', Newcommand)
    createCommand(parser, 'providecommand', Newcommand)

    createCommand(parser, 'newenvironment', Newenvironment)
    createCommand(parser, 'renewenvironment', Newenvironment)

    createCommand(parser, 'newtheorem', Newtheorem)

    createCommand(parser, 'def', DefCommand)
    createCommand(parser, 'edef', DefCommand)
    createCommand(parser, 'gdef', DefCommand)
    createCommand(parser, 'xdef', DefCommand)

    createCommand(parser, 'documentclass', Documentclass)
    createCommand(parser, 'documentstyle', Documentclass)

    createCommand(parser, 'input', InputCommand)

    mathToken = texpy.Token(texpy.Token.Type.CHARACTER,
                            texpy.Token.CatCode.MATHSHIFT, '$')
    parser.setSymbol('\\(', texpy.TokenCommand(mathToken))
    parser.setSymbol('\\)', texpy.TokenCommand(mathToken))
    parser.setSymbol('\\[', texpy.TokenCommand(mathToken))
    parser.setSymbol('\\]', texpy.TokenCommand(mathToken))

