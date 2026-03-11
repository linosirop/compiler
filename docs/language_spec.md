# docs/language_spec.md

# Language Specification

## Lexical Grammar (EBNF)

token ::= whitespace* (keyword | identifier | literal | operator | separator) whitespace* ;

whitespace ::= ' ' | '\t' | '\n' | '\r' | comment ;

comment ::= '//' ~[\n]* '\n' | '/*' ~['*/']* '*/' ; // no nesting

keyword ::= 'if' | 'else' | 'while' | 'for' | 'int' | 'float' | 'bool' | 'return' | 'true' | 'false' | 'void' | 'struct' | 'fn' ;

identifier ::= [a-zA-Z] ([a-zA-Z0-9_])* ; // max 255 chars, case sensitive

literal ::= int_literal | float_literal | string_literal | bool_literal ;

int_literal ::= [0-9]+ ; // decimal, [0, 2^31-1]

float_literal ::= [0-9]* '.' [0-9]+ ;

string_literal ::= '"' ~["\n\"]* '"' ; // escapes optional

bool_literal ::= 'true' | 'false' ;

operator ::= '+' | '-' | '*' | '/' | '%' | '&' | '==' | '!=' | '<' | '<=' | '>' | '>=' | '+=' ;

separator ::= '(' | ')' | '{' | '}' | ';' | ',' | '=' ; // assign is separator? but in spec it's operator, adjust as per LANG-5

## Character Set: UTF-8 (ASCII subset for sprint 1)

## Keywords: listed above

## Identifiers: as EBNF, max 255, case sensitive

## Literals: as above, int [-2^31,2^31-1] but literals non-neg, float with '.', string no escapes yet, bool true/false

## Operators/Separators: as listed

## Whitespace/Comments: ignored, track positions, // and /* */ non-nested