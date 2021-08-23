import io
import sl_ast
import re

keywords = {
    "@render",
    "@fragment"
    "@kernel",
    "return",
    "pass",
    "passto"
}

non_kw_tokens = {
    'lcomment':re.compile(r'\/\/.*'),
    'identifier':re.compile(r'[_a-zA-Z]+'),
    'lbracket':re.compile(r'{'),
    'rbracket':re.compile(r'}'),
    'lparen':re.compile(r'\('),
    'rparen':re.compile(r'\)')
}

class Parser:
    stream:io.TextIOWrapper
    def __init__(self):
        return

    def setInputStream(self,stream:io.TextIOWrapper):
        self.stream = stream
    
    def parse() -> sl_ast.ParsedSourceFile:
    
        return