parser grammar TypeScriptParserANTLR;

options {
	tokenVocab = TypeScriptLexerANTLR;
}

@parser::members 
{
    void setChannelToTokenIfEquals(size_t tokenType)
    {
        auto *bufferedTokenStream = (antlr4::BufferedTokenStream*)_input;
        auto prevToken = bufferedTokenStream->get(getCurrentToken()->getTokenIndex() - 1);
        if (prevToken && prevToken->getType() == tokenType && prevToken->getChannel() != 0)
        {
            ((antlr4::WritableToken*)prevToken)->setChannel(0);
            bufferedTokenStream->seek(bufferedTokenStream->index() - 1);
        }
    }

    void token(size_t tokenType)
    {
        setChannelToTokenIfEquals(tokenType);
    }
}

// Actual grammar start.
main
    : moduleBody EOF ;

moduleBody 
    : moduleItem* ;

moduleItem
    : statementListItem
    ;

statementListItem 
    : statement
    | declaration
    ;

declaration 
    : hoistableDeclaration
    | lexicalDeclaration
    ;

hoistableDeclaration
    : functionDeclaration
    ;    

lexicalDeclaration
    : (CONST_KEYWORD | LET_KEYWORD | VAR_KEYWORD) bindingList
    ;

bindingList
    : lexicalBinding (COMMA_TOKEN lexicalBinding)*
    ;

lexicalBinding
    : bindingIdentifier typeParameter? initializer?
    | bindingPattern initializer
    ;

functionDeclaration
    : FUNCTION_KEYWORD bindingIdentifier? OPENPAREN_TOKEN formalParameters? CLOSEPAREN_TOKEN typeParameter? OPENBRACE_TOKEN functionBody CLOSEBRACE_TOKEN ;

uniqueFormalParameters 
    : formalParameters 
    ;

formalParameters
    : functionRestParameter 
    | formalParameter (COMMA_TOKEN formalParameter)* (COMMA_TOKEN functionRestParameter)? ;    

formalParameter
    : IdentifierName QUESTION_TOKEN? typeParameter? initializer? ;    

typeParameter
    : COLON_TOKEN typeDeclaration ;    

initializer
    : EQUALS_TOKEN assignmentExpression ;  

typeDeclaration
    : ANY_KEYWORD
    | NUMBER_KEYWORD
    | BOOLEAN_KEYWORD
    | STRING_KEYWORD
    | BIGINT_KEYWORD 
    | VOID_KEYWORD ;    

functionRestParameter
    : DOTDOTDOT_TOKEN formalParameter ;

functionBody
    : functionStatementList ;    

functionStatementList
    : statementList ;

statementList
    : statementListItem* ;    

statement
    : emptyStatement
    | expressionStatement 
    | ifStatement
    | returnStatement
    | block
    ;    

testEndStatement
    : {token(LineTerminatorSequence);} (SEMICOLON_TOKEN | LineTerminatorSequence)
    ;

block
    : OPENBRACE_TOKEN statementList CLOSEBRACE_TOKEN ;

emptyStatement
    : SEMICOLON_TOKEN ;

expressionStatement
    : expression testEndStatement ;

ifStatement
    : IF_KEYWORD OPENPAREN_TOKEN expression CLOSEPAREN_TOKEN statement ELSE_KEYWORD statement // No need for statementTerminator as statement is teminated
    | IF_KEYWORD OPENPAREN_TOKEN expression CLOSEPAREN_TOKEN statement {getCurrentToken()->getType()!=ELSE_KEYWORD}?
    ;

returnStatement
    : RETURN_KEYWORD expression? testEndStatement ;

expression
    : assignmentExpression (COMMA_TOKEN assignmentExpression)* 
    ;   

exponentiationExpression
    : unaryExpression
    | updateExpression ASTERISKASTERISK_TOKEN exponentiationExpression
    ;      

multiplicativeExpression
    : exponentiationExpression
    | multiplicativeExpression multiplicativeOperator exponentiationExpression
    ;  

multiplicativeOperator
    : ASTERISK_TOKEN
    | SLASH_TOKEN
    | PERCENT_TOKEN
    ;

additiveExpression
    : multiplicativeExpression
    | additiveExpression PLUS_TOKEN multiplicativeExpression
    | additiveExpression MINUS_TOKEN multiplicativeExpression
    ;    

relationalExpression
    : shiftExpression
    | relationalExpression LESSTHAN_TOKEN shiftExpression
    | relationalExpression GREATERTHAN_TOKEN shiftExpression
    | relationalExpression LESSTHANEQUALS_TOKEN shiftExpression
    | relationalExpression GREATERTHANEQUALS_TOKEN shiftExpression
    | relationalExpression INSTANCEOF_KEYWORD shiftExpression
    | relationalExpression IN_KEYWORD shiftExpression
    ;

shiftExpression
    : additiveExpression
    | shiftExpression LESSTHANLESSTHAN_TOKEN additiveExpression
    | shiftExpression GREATERTHANGREATERTHAN_TOKEN additiveExpression
    | shiftExpression GREATERTHANGREATERTHANGREATERTHAN_TOKEN additiveExpression    
    ;

equalityExpression
    : relationalExpression 
    | equalityExpression EQUALSEQUALS_TOKEN relationalExpression
    | equalityExpression EXCLAMATIONEQUALS_TOKEN relationalExpression
    | equalityExpression EQUALSEQUALSEQUALS_TOKEN relationalExpression
    | equalityExpression EXCLAMATIONEQUALSEQUALS_TOKEN relationalExpression
    ;    

bitwiseANDExpression
    : equalityExpression 
    | bitwiseANDExpression AMPERSAND_TOKEN equalityExpression 
    ;

bitwiseXORExpression
    : bitwiseANDExpression 
    | bitwiseXORExpression CARET_TOKEN bitwiseANDExpression
    ;    

bitwiseORExpression
    : bitwiseXORExpression 
    | bitwiseORExpression BAR_TOKEN bitwiseXORExpression
    ;    

logicalANDExpression
    : bitwiseORExpression 
    | logicalANDExpression AMPERSANDAMPERSAND_TOKEN bitwiseORExpression
    ;

logicalORExpression
    : logicalANDExpression 
    | logicalORExpression BARBAR_TOKEN logicalANDExpression
    ;

coalesceExpression 
    : coalesceExpressionHead QUESTIONQUESTION_TOKEN bitwiseORExpression
    ;

coalesceExpressionHead 
    : coalesceExpression
    | bitwiseORExpression
    ;

shortCircuitExpression    
    : logicalORExpression 
    | coalesceExpression
    ;

conditionalExpression
    : shortCircuitExpression
    | shortCircuitExpression QUESTION_TOKEN assignmentExpression COLON_TOKEN assignmentExpression 
    ;

assignmentExpression 
    : conditionalExpression
    | yieldExpression
    | arrowFunction
    | asyncArrowFunction
    | leftHandSideExpression EQUALS_TOKEN assignmentExpression
    | leftHandSideExpression assignmentOperator assignmentExpression
    | leftHandSideExpression AMPERSANDAMPERSANDEQUALS_TOKEN assignmentExpression
    | leftHandSideExpression BARBAREQUALS_TOKEN assignmentExpression
    | leftHandSideExpression QUESTIONQUESTIONEQUALS_TOKEN assignmentExpression
    ;

assignmentOperator
    : ASTERISKEQUALS_TOKEN 
    | SLASHEQUALS_TOKEN 
    | PERCENTEQUALS_TOKEN 
    | PLUSEQUALS_TOKEN 
    | MINUSEQUALS_TOKEN 
    | LESSTHANLESSTHANEQUALS_TOKEN 
    | GREATERTHANGREATERTHANEQUALS_TOKEN 
    | GREATERTHANGREATERTHANGREATERTHANEQUALS_TOKEN 
    | AMPERSANDEQUALS_TOKEN 
    | CARETEQUALS_TOKEN 
    | BAREQUALS_TOKEN 
    | ASTERISKASTERISKEQUALS_TOKEN
    ;

unaryExpression
    : updateExpression
    | DELETE_KEYWORD unaryExpression
    | VOID_KEYWORD unaryExpression
    | TYPEOF_KEYWORD unaryExpression
    | PLUS_TOKEN unaryExpression
    | MINUS_TOKEN unaryExpression
    | TILDE_TOKEN unaryExpression
    | EXCLAMATION_TOKEN unaryExpression
    | awaitExpression
    ;

updateExpression 
    : leftHandSideExpression
    | leftHandSideExpression PLUSPLUS_TOKEN
    | leftHandSideExpression MINUSMINUS_TOKEN
    | PLUSPLUS_TOKEN unaryExpression
    | MINUSMINUS_TOKEN unaryExpression
    ;

leftHandSideExpression    
    : newExpression
    | callExpression
    | optionalExpression
    ;

yieldExpression
    : YIELD_KEYWORD ASTERISK_TOKEN? assignmentExpression? ;    

newExpression
    : memberExpression
    | NEW_KEYWORD newExpression 
    ;

callExpression
    : coverCallExpressionAndAsyncArrowHead
    | callExpression arguments 
    ;

memberExpression    
    : primaryExpression
    | memberExpression DOT_TOKEN IdentifierName 
    ;

primaryExpression
    : THIS_KEYWORD
    | literal
    | identifierReference 
    | coverParenthesizedExpressionAndArrowParameterList 
    ;

coverParenthesizedExpressionAndArrowParameterList
    : OPENPAREN_TOKEN (expression COMMA_TOKEN?)? (DOTDOTDOT_TOKEN (bindingIdentifier|bindingPattern))? CLOSEPAREN_TOKEN
    ;

optionalExpression
    : memberExpression optionalChain
    ;    

optionalChain
    : QUESTIONDOT_TOKEN IdentifierName
    ;    

nullLiteral
    : NULL_KEYWORD ;

undefinedLiteral
    : UNDEFINED_KEYWORD ;

booleanLiteral
    : TRUE_KEYWORD
    | FALSE_KEYWORD ;

literal
    : nullLiteral
    | undefinedLiteral
    | booleanLiteral
    | numericLiteral
    | StringLiteral ;

numericLiteral 
    : DecimalLiteral
    | DecimalIntegerLiteral
    | DecimalBigIntegerLiteral
    | BinaryBigIntegerLiteral
    | OctalBigIntegerLiteral
    | HexBigIntegerLiteral ;        

identifierReference
    : identifier ;

bindingIdentifier    
    : identifier
    | YIELD_KEYWORD
    | AWAIT_KEYWORD 
    ;

bindingPattern
    : objectBindingPattern
    | arrayBindingPattern
    ;

objectBindingPattern
    : OPENBRACE_TOKEN CLOSEBRACE_TOKEN
// TODO    
    ;    

arrayBindingPattern
    : OPENBRACKET_TOKEN CLOSEBRACKET_TOKEN
// TODO    
    ;    

identifier
    : IdentifierName ; // but not ReservedWord 

arguments
    : OPENPAREN_TOKEN argumentList? COMMA_TOKEN? CLOSEPAREN_TOKEN  
    ;

argumentList
    : argumentListItem (COMMA_TOKEN argumentListItem)*
    ;

argumentListItem
    : DOTDOTDOT_TOKEN? assignmentExpression ;

propertyName 
    : literalPropertyName
    | computedPropertyName
    ;

literalPropertyName
    : IdentifierName
    | StringLiteral
    | numericLiteral
    ;

computedPropertyName 
    : OPENBRACKET_TOKEN assignmentExpression CLOSEBRACKET_TOKEN ;

arrowFunction 
    : arrowParameters EQUALSGREATERTHAN_TOKEN conciseBody ;

arrowParameters 
    : bindingIdentifier
    | coverParenthesizedExpressionAndArrowParameterList
    ;

conciseBody 
    : {_input->LA(1)->getType() != OPENBRACE_TOKEN}? expressionBody
    | OPENBRACE_TOKEN functionBody CLOSEBRACE_TOKEN
    ;

expressionBody 
    : assignmentExpression ;

asyncFunctionDeclaration
    : ASYNC_KEYWORD FUNCTION_KEYWORD bindingIdentifier OPENPAREN_TOKEN formalParameters CLOSEPAREN_TOKEN OPENBRACE_TOKEN asyncFunctionBody CLOSEBRACE_TOKEN
    | ASYNC_KEYWORD FUNCTION_KEYWORD OPENPAREN_TOKEN formalParameters CLOSEPAREN_TOKEN OPENBRACE_TOKEN asyncFunctionBody CLOSEBRACE_TOKEN
    ;

asyncArrowFunction 
    : ASYNC_KEYWORD asyncArrowBindingIdentifier EQUALSGREATERTHAN_TOKEN asyncConciseBody
    | coverCallExpressionAndAsyncArrowHead EQUALSGREATERTHAN_TOKEN asyncConciseBody
    ;

asyncFunctionExpression 
    : ASYNC_KEYWORD FUNCTION_KEYWORD bindingIdentifier OPENPAREN_TOKEN formalParameters CLOSEPAREN_TOKEN OPENBRACE_TOKEN asyncFunctionBody CLOSEBRACE_TOKEN 
    ;

asyncMethod 
    : ASYNC_KEYWORD propertyName OPENPAREN_TOKEN uniqueFormalParameters CLOSEPAREN_TOKEN OPENBRACE_TOKEN asyncFunctionBody CLOSEBRACE_TOKEN
    ;

asyncFunctionBody 
    : functionBody 
    ;

awaitExpression 
    : AWAIT_KEYWORD unaryExpression 
    ;

asyncConciseBody 
    : {_input->LA(1)->getType() != OPENBRACE_TOKEN}? expressionBody
    | OPENBRACE_TOKEN asyncFunctionBody CLOSEBRACE_TOKEN
    ;

asyncArrowBindingIdentifier
    : bindingIdentifier ;

coverCallExpressionAndAsyncArrowHead 
    : memberExpression arguments ;