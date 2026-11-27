Grammar for the Cam Language:

Note: Numbers are either double-precision floating point,
      or unsigned 32 bit integers

digit : [0-9]
alpha : [A-Za-z_]

comment: # .* \n

NUMBER : {digit}+ ("." {digit}+ (("e" | "E")("+" | "-")?{digit}+)?)?

identifier : {alpha}{(digit | alpha)}*

STRING : " .* "

type : "string" | "i32" | "f64" | "bool" | "unit"

compare : "<" | ">" | "<=" | ">="
equal : "==" | "!="
assign : "=" | "+=" | "-=" | "*=" | "/="

type   -> identifier
        ; # can change this to allow for array types etc

params -> identifier ":" type ( "," identifier ":" type )*
        ;
args   -> expression ( "," expression )*
        ;



file           -> ( declaration )* EOF
                ;

declaration    -> ( "var" | "const" ) identifier "=" expression ";"
                | "var" identifier ":" type ";"
                | "fn" identifier "(" params? ")" type block
                ;

block          -> "{" ( statement )* "}"
                ;

statement      -> declaration
                | "while" expression block
                | ifStatement
                | assignment ";"
                | "return" expression ";"
                | block
                | expression ";"
                ;



expression     -> orexpr
                ;

assignment     -> identifier assign expression
                ;

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
call           -> identifier ( "(" args? ")" )
                | primary # Todo: This is where [], . etc come

primary        -> NUMBER
                | STRING
                | identifier
                | "true"
                | "false"
                | "(" expression ")"
                ;

Todo: Type inference/checking
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
14 primary


Next thing to do:
- Binary and Unary type checking
- Add a 'print all known types'
- Add stringify ability to types
- Fix the grammar (i.e. var declarations)
- Handle constants