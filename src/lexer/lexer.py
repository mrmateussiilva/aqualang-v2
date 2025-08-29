#!/usr/bin/env python3
"""
Lexer para a linguagem de programação Aqua
Analisa o código fonte e gera tokens para o parser
"""

from enum import Enum, auto
from typing import List, Optional, Tuple
import re

class TokenType(Enum):
    # Palavras-chave
    FUNC = auto()
    LET = auto()
    IMPORT = auto()
    SPAWN = auto()
    LOOP = auto()
    MATCH = auto()
    CASE = auto()
    BREAK = auto()
    RETURN = auto()
    
    # Identificadores e literais
    IDENTIFIER = auto()
    STRING = auto()
    NUMBER = auto()
    BOOLEAN = auto()
    
    # Operadores
    PLUS = auto()
    MINUS = auto()
    MULTIPLY = auto()
    DIVIDE = auto()
    ASSIGN = auto()
    EQUALS = auto()
    NOT_EQUALS = auto()
    GREATER = auto()
    LESS = auto()
    GREATER_EQUAL = auto()
    LESS_EQUAL = auto()
    
    # Delimitadores
    LPAREN = auto()
    RPAREN = auto()
    LBRACKET = auto()
    RBRACKET = auto()
    COMMA = auto()
    SEMICOLON = auto()
    COLON = auto()
    ARROW = auto()
    DOT = auto()
    
    # Especiais
    NEWLINE = auto()
    INDENT = auto()
    DEDENT = auto()
    EOF = auto()

class Token:
    def __init__(self, type: TokenType, value: str, line: int, column: int):
        self.type = type
        self.value = value
        self.line = line
        self.column = column
    
    def __repr__(self):
        return f"Token({self.type}, '{self.value}', line={self.line}, col={self.column})"

class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.position = 0
        self.line = 1
        self.column = 1
        self.indent_stack = [0]
        self.tokens = []
        
        # Padrões para tokens
        self.patterns = [
            # Palavras-chave
            (r'\bfunc\b', TokenType.FUNC),
            (r'\blet\b', TokenType.LET),
            (r'\bimport\b', TokenType.IMPORT),
            (r'\bspawn\b', TokenType.SPAWN),
            (r'\bloop\b', TokenType.LOOP),
            (r'\bmatch\b', TokenType.MATCH),
            (r'\bcase\b', TokenType.CASE),
            (r'\bbreak\b', TokenType.BREAK),
            (r'\breturn\b', TokenType.RETURN),
            (r'\btrue\b', TokenType.BOOLEAN),
            (r'\bfalse\b', TokenType.BOOLEAN),
            
            # Números
            (r'\d+\.\d+', TokenType.NUMBER),  # Float
            (r'\d+', TokenType.NUMBER),        # Integer
            
            # Strings
            (r'"[^"]*"', TokenType.STRING),
            (r"'[^']*'", TokenType.STRING),
            
            # Operadores
            (r'\+', TokenType.PLUS),
            (r'-', TokenType.MINUS),
            (r'\*', TokenType.MULTIPLY),
            (r'/', TokenType.DIVIDE),
            (r'=', TokenType.ASSIGN),
            (r'==', TokenType.EQUALS),
            (r'!=', TokenType.NOT_EQUALS),
            (r'>', TokenType.GREATER),
            (r'<', TokenType.LESS),
            (r'>=', TokenType.GREATER_EQUAL),
            (r'<=', TokenType.LESS_EQUAL),
            (r'=>', TokenType.ARROW),
            
            # Delimitadores
            (r'\(', TokenType.LPAREN),
            (r'\)', TokenType.RPAREN),
            (r'\[', TokenType.LBRACKET),
            (r'\]', TokenType.RBRACKET),
            (r',', TokenType.COMMA),
            (r';', TokenType.SEMICOLON),
            (r':', TokenType.COLON),
            (r'\.', TokenType.DOT),
            
            # Identificadores
            (r'[a-zA-Z_][a-zA-Z0-9_]*', TokenType.IDENTIFIER),
        ]
        
        # Compilar padrões regex
        self.compiled_patterns = [(re.compile(pattern), token_type) 
                                 for pattern, token_type in self.patterns]
    
    def tokenize(self) -> List[Token]:
        """Analisa o código fonte e retorna uma lista de tokens"""
        while self.position < len(self.source):
            # Pular espaços em branco
            if self.skip_whitespace():
                continue
            
            # Pular comentários
            if self.skip_comments():
                continue
            
            # Processar nova linha e indentação
            if self.source[self.position] == '\n':
                self.handle_newline()
                continue
            
            # Tentar fazer match com padrões
            token = self.match_patterns()
            if token:
                self.tokens.append(token)
                continue
            
            # Caractere não reconhecido
            raise SyntaxError(f"Caractere não reconhecido '{self.source[self.position]}' "
                           f"na linha {self.line}, coluna {self.column}")
        
        # Adicionar tokens de indentação finais
        self.handle_final_indentation()
        
        # Adicionar EOF
        self.tokens.append(Token(TokenType.EOF, "", self.line, self.column))
        
        return self.tokens
    
    def skip_whitespace(self) -> bool:
        """Pula espaços em branco e retorna True se pulou algum"""
        skipped = False
        while (self.position < len(self.source) and 
               self.source[self.position].isspace() and 
               self.source[self.position] != '\n'):
            self.position += 1
            self.column += 1
            skipped = True
        return skipped
    
    def skip_comments(self) -> bool:
        """Pula comentários e retorna True se pulou algum"""
        if (self.position < len(self.source) and 
            self.source[self.position] == '#'):
            # Comentário de linha única
            while (self.position < len(self.source) and 
                   self.source[self.position] != '\n'):
                self.position += 1
                self.column += 1
            return True
        return False
    
    def handle_newline(self):
        """Processa uma nova linha e gerencia indentação"""
        self.tokens.append(Token(TokenType.NEWLINE, "\n", self.line, self.column))
        self.position += 1
        self.line += 1
        self.column = 1
        
        # Calcular indentação da próxima linha
        indent_level = 0
        while (self.position < len(self.source) and 
               self.source[self.position] == ' '):
            indent_level += 1
            self.position += 1
            self.column += 1
        
        # Gerar tokens de indentação
        current_indent = self.indent_stack[-1]
        if indent_level > current_indent:
            self.indent_stack.append(indent_level)
            self.tokens.append(Token(TokenType.INDENT, "", self.line, self.column))
        elif indent_level < current_indent:
            while self.indent_stack and self.indent_stack[-1] > indent_level:
                self.indent_stack.pop()
                self.tokens.append(Token(TokenType.DEDENT, "", self.line, self.column))
    
    def match_patterns(self) -> Optional[Token]:
        """Tenta fazer match com os padrões de tokens"""
        for pattern, token_type in self.compiled_patterns:
            match = pattern.match(self.source, self.position)
            if match:
                value = match.group(0)
                token = Token(token_type, value, self.line, self.column)
                self.position += len(value)
                self.column += len(value)
                return token
        return None
    
    def handle_final_indentation(self):
        """Adiciona tokens de dedentação finais"""
        while len(self.indent_stack) > 1:
            self.indent_stack.pop()
            self.tokens.append(Token(TokenType.DEDENT, "", self.line, self.column))

def lex(source: str) -> List[Token]:
    """Função conveniente para tokenizar código fonte"""
    lexer = Lexer(source)
    return lexer.tokenize()

if __name__ == "__main__":
    # Exemplo de uso
    source_code = '''
func main()
    let x = 42
    let message = "Olá, Aqua!"
    io.println(message)
    io.println(x)
'''
    
    tokens = lex(source_code)
    for token in tokens:
        print(token)
