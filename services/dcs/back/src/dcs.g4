grammar dcs;

prog: (constantDefinition | functionDefinition)+ EOF;

constantDefinition: id ASSIGN constantValue SEMICOLON;

functionDefinition: FUN id argumentsDefinition statementsList;

argumentsDefinition: LEFT_PAREN (id (COMMA id)*)? RIGHT_PAREN;

statementsList: LEFT_BRACE statement+ RIGHT_BRACE;

statement: conditionalStatement | ((returnStatement | assignStatement) SEMICOLON);

returnStatement: RETURN expression;

conditionalStatement: IF LEFT_PAREN expression (LESS | GREAT | LE | GE | EQ | NEQ) expression RIGHT_PAREN statementsList (ELSE statementsList)?;

assignStatement: id ASSIGN expression;

expression: additiveExpression; 

additiveExpression: multiplicativeExpression ((PLUS | MINUS) multiplicativeExpression)*;

multiplicativeExpression: unaryExpression ((MUL | DIV) unaryExpression)*;

unaryExpression: id | constantValue | functionCall | parenthesisExpression;

parenthesisExpression: (LEFT_PAREN expression RIGHT_PAREN);

functionCall: id LEFT_PAREN (expression (COMMA expression)*)? RIGHT_PAREN;

constantValue: NUMBER;

id: NAME;

ASSIGN:'=';
SEMICOLON: ';';
LEFT_PAREN: '(';
RIGHT_PAREN: ')';
LEFT_BRACE: '{';
RIGHT_BRACE: '}';
FUN: 'fun';
RETURN: 'return';
IF: 'if';
ELSE: 'else';
PLUS: '+';
MINUS: '-';
MUL: '*';
DIV: '/';
COMMA: ',';
LESS: '<';
GREAT: '>';
EQ: '==';
NEQ: '!=';
LE: '<=';
GE: '>=';
NAME: [a-zA-Z_] [a-zA-Z0-9_]*;
NUMBER: ('+' | '-')? ([0-9]* '.' [0-9]*) | ([0-9]+ '.'?);
WS: [ \n\t\r]+ -> skip;
