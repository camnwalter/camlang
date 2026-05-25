Grammar for the Cam Language:

Note: All numbers are double-precision floating point

digit : [0-9]
alpha : [A-Za-z_]

comment: # .* \n

NUMBER : {digit}+ "." {digit}+ (("e" | "E")("+" | "-")?{digit}+)?

identifier : {alpha}{(digit | alpha)}*

STRING : " .* "

compare : "<" | ">" | "<=" | ">="
equal : "=" | "!="

block          -> "{" ( statement )* "}"

statement      -> expression ";"
                | "{" ( statement )* "}"

expression     -> equality ;

equality       -> comparison ( equal comparison )* ;
comparison     -> term ( compare term )* ;
term           -> factor ( ( "-" | "+" ) factor )* ;
factor         -> unary ( ( "/" | "\*" ) unary )* ;
unary          -> ("-" | "+") exponent
                | "!" unary
                | exponent ;
exponent       -> primary ( ^ exponent )* ;
primary        -> NUMBER
                | STRING
                | identifier
                | "true"
                | "false"
                | "(" expression ")";
