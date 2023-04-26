#pragma once

#include <memory>
#include <tuple>

#include "AST.h"
#include "lexer/TokenStream.h"


namespace minisolc {

class Parser {
public:
	Parser(TokenStream& source): m_source(source) {}
	bool parse();
	void Dump() const {
		if (m_root)
			m_root->Dump();
	}

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
	bool matchGet(std::string val, std::string& out) {
		bool res = m_source.curVal() == val;
		out = m_source.curVal();
		m_source.advance();
		return res;
	}

	bool parseSourceUnit(std::unique_ptr<BaseAST>&);
	bool parseContractDefinition(std::unique_ptr<BaseAST>&);
	bool parseContractPart(std::unique_ptr<BaseAST>&);
	bool parseStateVariableDeclaration(std::unique_ptr<BaseAST>&);
	bool parseFunctionDefinition(std::unique_ptr<BaseAST>&);
	bool parseParameterList(std::unique_ptr<BaseAST>&);
	bool parseTypeName(std::unique_ptr<BaseAST>&);
	bool parseElementaryTypeName(std::unique_ptr<BaseAST>&);
	bool parseBlock(std::unique_ptr<BaseAST>&);
	bool parseStatement(std::unique_ptr<BaseAST>&);
	bool parseReturn(std::unique_ptr<BaseAST>&);
	bool parseExpression(std::unique_ptr<BaseAST>&);
	bool parsePrimaryExpression(std::unique_ptr<BaseAST>&);

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
/// FunctionDefinition = 'function' Identifier ParameterList ('pure' | 'view' | 'payable' | 'external' | 'public' |
/// 'internal' | 'private')* ('returns' TypeName)? Block
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

// // Precedence by order (see github.com/ethereum/solidity/pull/732)
// Expression
//   = Expression ('++' | '--')
//   | NewExpression
//   | IndexAccess
//   | MemberAccess
//   | FunctionCall
//   | '(' Expression ')'
//   | ('!' | '~' | 'delete' | '++' | '--' | '+' | '-') Expression
//   | Expression '**' Expression
//   | Expression ('*' | '/' | '%') Expression
//   | Expression ('+' | '-') Expression
//   | Expression ('<<' | '>>') Expression
//   | Expression '&' Expression
//   | Expression '^' Expression
//   | Expression '|' Expression
//   | Expression ('<' | '>' | '<=' | '>=') Expression
//   | Expression ('==' | '!=') Expression
//   | Expression '&&' Expression
//   | Expression '||' Expression
//   | Expression '?' Expression ':' Expression
//   | Expression ('=' | '|=' | '^=' | '&=' | '<<=' | '>>=' | '+=' | '-=' | '*=' | '/=' | '%=') Expression
//   | PrimaryExpression

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