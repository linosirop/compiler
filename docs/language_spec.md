# MiniCompiler Language Specification

This document describes the lexical grammar of the MiniCompiler source language.
The source language is a simplified C-like language used for an educational compiler front-end.

## Character Set and Encoding

Source files are treated as UTF-8 text. The language grammar itself uses the ASCII subset for keywords, identifiers, operators, delimiters, and literals.

## Whitespace and Comments

Whitespace is ignored except for line and column tracking.

```ebnf
whitespace          ::= " " | "\t" | "\n" | "\r" ;
line_comment       ::= "//" { any_character_except_newline } ("\n" | end_of_file) ;
block_comment      ::= "/*" { any_character } "*/" ;
comment            ::= line_comment | block_comment ;
```

Block comments are not nested. Unterminated block comments are lexical errors. Windows line endings (`\r\n`) and Unix line endings (`\n`) must be handled correctly.

## Keywords

The following words are reserved and cannot be used as identifiers:

```text
if else while for int float bool return void struct fn
```

Boolean constants are reserved boolean literals:

```text
true false
```

The lexer emits `true` and `false` as `BOOL_LITERAL`, not as normal identifiers.

## Identifiers

```ebnf
identifier ::= letter { letter | digit | "_" } ;
letter     ::= "A" ... "Z" | "a" ... "z" ;
digit      ::= "0" ... "9" ;
```

Rules:

- identifiers are case-sensitive;
- identifiers must start with a letter;
- underscore is allowed only after the first character;
- maximum identifier length is 255 characters;
- examples of valid identifiers: `x`, `counter`, `my_value`, `Point2`;
- examples of invalid identifiers: `_x`, `2value`.

## Literals

```ebnf
literal       ::= int_literal | float_literal | string_literal | bool_literal ;
int_literal   ::= digit { digit } ;
float_literal ::= digit { digit } "." digit { digit } ;
string_literal ::= '"' { string_character } '"' ;
bool_literal  ::= "true" | "false" ;
```

### Integer literals

Integer literals are decimal. The lexer stores integer literals as 32-bit signed integer values. The accepted lexical range for a non-negative integer token is:

```text
0 ... 2147483647
```

Negative values are represented syntactically by the unary minus operator followed by an integer literal.

### Floating-point literals

Floating-point literals must contain a decimal point and at least one digit after the point.

Valid examples:

```text
0.5
1.0
42.25
```

Invalid examples:

```text
1.
.5
```

### String literals

String literals are enclosed in double quotes. Newline characters inside string literals are lexical errors.

```text
"hello"
"MiniCompiler"
```

Escape sequences are not required for the current sprint.

### Boolean literals

```text
true
false
```

## Operators

```ebnf
arithmetic_operator ::= "+" | "-" | "*" | "/" | "%" ;
relational_operator ::= "<" | "<=" | ">" | ">=" ;
equality_operator   ::= "==" | "!=" ;
logical_operator    ::= "&&" | "||" | "!" ;
assignment_operator ::= "=" | "+=" | "-=" | "*=" | "/=" ;
function_arrow      ::= "->" ;
```

The single `&` token exists in the token set, but logical conjunction is written as `&&`.

## Delimiters

```ebnf
delimiter ::= "(" | ")" | "{" | "}" | "[" | "]" | ";" | "," | ":" ;
```

## Token Categories

The lexer produces tokens from the following categories:

- keywords;
- identifiers;
- integer literals;
- floating-point literals;
- string literals;
- boolean literals;
- operators;
- delimiters;
- end-of-file marker.

Each token stores:

- token type;
- lexeme;
- literal value when applicable;
- 1-indexed line number;
- 1-indexed column number.

## Lexical Error Handling

The lexer reports lexical errors with line and column information. Examples:

- invalid characters: `@`, unexpected single `|`;
- unterminated string literals;
- unterminated block comments;
- malformed floating-point literals such as `1.`;
- integer literals outside the accepted range;
- identifiers longer than 255 characters;
- identifiers that start with `_`.

After reporting a lexical error, the lexer attempts basic recovery and continues scanning.
