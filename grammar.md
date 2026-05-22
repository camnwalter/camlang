Grammar for the Cam Language:

digit : [0-9]
alpha : [A-Za-z_]

comment: #.*\n

integer : {digit}+

identifier : {alpha}{(digit | alpha)}*

STRING : " .* "



expression     -> equality ;
equality       -> comparison ;
comparison     -> term ;
term           -> factor ( ( "-" | "+" ) factor )* ;
factor         -> unary ( ( "/" | "*" ) unary )* ;
unary          -> unary
               | primary ;
primary        -> NUMBER | STRING ;