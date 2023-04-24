#pragma once

#include <memory>
#include <tuple>

#include "lexer/TokenStream.h"
#include "AST.h"

namespace minisolc {

class Parser {
public:
	Parser(TokenStream& source): m_source(source) {}
    bool parse();

private:
    bool match(Token tok) {
        bool res = m_source.curTok() == tok;
        m_source.advance();
        return res;
    }
    bool matchGet(Token tok, std::string& val) {
        bool res = m_source.curTok() == tok;
        val = m_source.curVal();
        m_source.advance();
        return res;
    }
    bool match(std::string val) {
        bool res = m_source.curVal() == val;
        m_source.advance();
        return res;
    }

    bool parseSourceUnit(std::unique_ptr<BaseAST>&);
    bool parseContractDefinition(std::unique_ptr<BaseAST>&);

	TokenStream& m_source;
    std::unique_ptr<BaseAST> m_root;
};

}

/// @{
/// @name ENBF
/// SourceUnit = ContractDefinition
/// ContractDefinition = 'contract' Identifier '{' ContractPart* '}'
/// ContractPart = StateVariableDeclaration | FunctionDefinition
/// StateVariableDeclaration = TypeName Identifier ('=' Expression)? ';'
/// FunctionDefinition = 'function' Identifier ParameterList (StateMutability | 'external' | 'public' | 'internal' | 'private')* ('returns' TypeName)? Block
/// StateMutability = 'pure' | 'const' | 'view' | 'payable'
///
/// ParameterList = '(' (TypeName Identifier (',' TypeName Identifier)*)? ')'
/// TypeName = ElementaryTypeName
/// ElementaryTypeName = 'address' | 'bool' | 'string' | 'var' | Int | Uint
/// Int = 'int' ('8' | '16' | '32' | '64' | '128' | '256')?
/// UInt = 'uint' ('8' | '16' | '32' | '64' | '128' | '256')?
/// 
/// Block = '{' Statement* '}'
/// Statement = Return ';'
/// Return = 'return' Expression?
/// Expression = PrimaryExpression
/// PrimaryExpression = BooleanLiteral
///                   | NumberLiteral
///                   | StringLiteral
///                   | Identifier
/// BooleanLiteral = 'true' | 'false'
/// NumberLiteral = ( HexNumber | DecimalNumber ) (' ' NumberUnit)?
/// NumberUnit = 'wei' | 'szabo' | 'finney' | 'ether'
///            | 'seconds' | 'minutes' | 'hours' | 'days' | 'weeks' | 'years'
/// StringLiteral = '"' ([^"\r\n\\] | '\\' .)* '"'
/// Identifier = [a-zA-Z_] [a-zA-Z_0-9]*
/// @}