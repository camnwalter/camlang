Grammar for the Cam Language:

Note: All numbers are double-precision floating point

digit : [0-9]
alpha : [A-Za-z_]

comment: # .* \n

NUMBER : {digit}+ "." {digit}+ (("e" | "E")("+" | "-")?{digit}+)?

identifier : {alpha}{(digit | alpha)}*

STRING : " .* "

type : "string" | "i32" | "f64" | "bool"

compare : "<" | ">" | "<=" | ">="
equal : "==" | "!="
assign : "=" | "+=" | "-=" | "*=" | "/="

params : identifier ( "," identifier )* ","?
args : expression ( "," expression )* ","?

file           -> ( declaration )*

block          -> "{" ( declaration )* "}"
exprStatement  -> expression ";"

declaration    -> "var" identifier "=" exprStatement
                | "const" identifier "=" exprStatement
                | "fn" identifier "(" params? ")" block
                | statement

statement      -> block
                | ifStatement
                | "while" expression block
                | "return" exprStatement
                | exprStatement
                | ";"

ifStatement    -> "if" expression block elsePart

elsePart       -> "else" ifStatement
                | "else" block
                | ""


expression     -> assignment
                | orexpr ;
assignment     -> identifier ( assign expression )+ ;

orexpr         -> andexpr ( "or" andexpr )* ;
andexpr        -> bitor ( "and" bitor )* ;
bitor          -> xor ( "|" xor )* ;
xor            -> bitand ( "xor" bitand )* ;
bitand         -> equality ( "&" equality )* ;
equality       -> comparison ( equal comparison )* ;
comparison     -> shifts ( compare shifts )* ;
shifts         -> term ( "<<" | ">>" term )* ;
term           -> factor ( ( "-" | "+" ) factor )* ;
factor         -> unary ( ( "/" | "\*" | "mod" ) unary )* ;
unary          -> ( "-" | "+" | "!" | "~" ) unary
                | call ;
call           -> primary ( "(" args? ")" )* # Todo: This is where [], . etc come
primary        -> NUMBER
                | STRING
                | identifier
                | "true"
                | "false"
                | "(" expression ")";

Todo: Type inference/checking
Todo: Symbol table
Todo: Records


Precedence Table (low to high):
1 =, +=, -=, *=, /=
2 or
3 and
4 |
5 xor
6 &
7 ==, !=
8 <, <=, >, >=
9 <<, >>
10 +, -
11 *, /, mod
12 - (unary), + (unary), !, ~
13 call
14 string, number, bool, identifier, parenthesized_expr 