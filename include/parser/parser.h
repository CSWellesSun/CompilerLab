#pragma once

#include <memory>
#include <tuple>

#include "AST.h"
#include "lexer/TokenStream.h"
#include "common/Error.h"

namespace minisolc {

class Parser {
public:
	Parser(TokenStream& source): m_source(source) {}
	void parse();
	void Dump() const {
		if (m_root)
			m_root->Dump();
	}

private:
	bool peekCur(Token tok) {
		if (curTok() == Token::EOS)
			return false;
		bool res = curTok() == tok;
		return res;
	}
	bool peekCur(bool (*func)(Token)) {
		if (curTok() == Token::EOS)
			return false;
		bool res = func(curTok());
		return res;
	}
	bool match(Token tok) {
		if (curTok() == Token::EOS)
			return false;
		bool res = curTok() == tok;
		if (res)
			advance();
		return res;
	}
	bool match(bool (*func)(Token)) {
		if (curTok() == Token::EOS)
			return false;
		bool res = func(curTok());
		if (res)
			advance();
		return res;
	};
	bool matchGet(Token tok, std::string& val) {
		val = curVal();
		return match(tok);
	}
	bool matchGet(bool (*func)(Token), std::string& val) {
		val = curVal();
		return match(func);
	}
	
	bool expect(Token tok) {
		if (match(tok)) return true;
		throw UnexpectedToken(curLine(), curTok(), tok);
		return false;
	}
	bool expect(bool (*func)(Token)) {
		if (match(func)) return true;
		throw UnexpectedToken(curLine(), curTok(), func);
		return false;
	}
	bool expectGet(Token tok, std::string& val) {
		if (matchGet(tok, val)) return true;
		throw UnexpectedToken(curLine(), curTok(), tok);
		return false;
	}
	bool expectGet(bool (*func)(Token), std::string& val) {
		if (matchGet(func, val)) return true;
		throw UnexpectedToken(curLine(), curTok(), func);
		return false;
	}

	Token curTok() const { return m_source.curTok(); }
	std::string curVal() const { return m_source.curVal(); }
	std::string curLine() const { return m_source.curLine(); }
	void advance() { m_source.advance(); }
	bool eof() const { return m_source.curTok() == Token::EOS; }

	std::unique_ptr<SourceUnit> parseSourceUnit();
	std::unique_ptr<ContractDefinition> parseContractDefinition();
	std::unique_ptr<VariableDeclaration> parseVariableDeclaration(bool end);
	std::unique_ptr<FunctionDefinition> parseFunctionDefinition();
	std::unique_ptr<ParameterList> parseParameterList();
	std::unique_ptr<TypeName> parseTypeName();
	std::unique_ptr<Block> parseBlock();
	std::unique_ptr<Statement> parseStatement();
	std::unique_ptr<Return> parseReturn();
	std::unique_ptr<Expression> parseExpression();
	std::unique_ptr<PrimaryExpression> parsePrimaryExpression();

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