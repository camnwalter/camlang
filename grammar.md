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
                | "fn" identifier "(" params? ")" block
                | statement

statement      -> block
                | ifStatement
                | "while" expression block
                | "return" exprStatement
                | exprStatement

ifStatement    -> "if" expression block
                | "if" expression block else block


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
unary          -> ( "-" | "+" ) exponent
                | ( "!" | "~" ) unary        # Todo: Rewrite as a while loop?
                | exponent ;
exponent       -> call ( ^ call )* ;
call           -> primary ( "(" args? ")" )* # Todo: This is where [], . etc come
primary        -> NUMBER
                | STRING
                | identifier
                | "true"
                | "false"
                | "(" expression ")";


Todo: Take a look at smart pointers