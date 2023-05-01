#pragma once

#include <memory>
// #include <tuple>
#include <functional>

#include "AST.h"
#include "lexer/TokenStream.h"
#include "common/Error.h"

namespace minisolc {

class Parser {
public:
	Parser(TokenStream& source): m_source(source) {}
	void parse();
	void Dump() const {
		if (m_root) {
			m_root->Dump(0, 0);
		}
	}

private:
	bool peekCur(Token tok) {
		if (curTok() == Token::EOS)
			return false;
		bool res = curTok() == tok;
		return res;
	}
	bool peekCur(std::function<bool(Token)> func) {
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
	bool match(std::function<bool(Token)> func) {
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
	bool matchGet(std::function<bool(Token)> func, std::string& val) {
		val = curVal();
		return match(func);
	}
	
	bool expect(Token tok) {
		if (match(tok)) return true;
		throw UnexpectedToken(curTokInfo(), tok);
		return false;
	}
	bool expect(std::function<bool(Token)> func) {
		if (match(func)) return true;
		throw UnexpectedToken(curTokInfo(), func);
		return false;
	}
	bool expectGet(Token tok, std::string& val) {
		if (matchGet(tok, val)) return true;
		throw UnexpectedToken(curTokInfo(), tok);
		return false;
	}
	bool expectGet(std::function<bool(Token)> func, std::string& val) {
		if (matchGet(func, val)) return true;
		throw UnexpectedToken(curTokInfo(), func);
		return false;
	}

	Token curTok() const { return m_source.curTok(); }
	std::string curVal() const { return m_source.curVal(); }
	TokenInfo curTokInfo() const { return m_source.curTokInfo(); }
	void advance() { m_source.advance(); }
	bool eof() const { return m_source.curTok() == Token::EOS; }

	std::shared_ptr<SourceUnit> parseSourceUnit();
	std::shared_ptr<ContractDefinition> parseContractDefinition();
	std::shared_ptr<VariableDeclaration> parseVariableDeclaration(bool end);
	std::shared_ptr<FunctionDefinition> parseFunctionDefinition();
	std::shared_ptr<ParameterList> parseParameterList();
	std::shared_ptr<TypeName> parseTypeName();
	std::shared_ptr<Block> parseBlock();
	std::shared_ptr<Statement> parseStatement();
	std::shared_ptr<ReturnStatement> parseReturn();
	std::shared_ptr<Expression> parseExpression(
		std::shared_ptr<Expression> const& partiallyParsedExpression = std::shared_ptr<Expression>()
	);
	std::shared_ptr<Expression> parseBinaryExpression(
		int minPrecedence = 4,
		std::shared_ptr<Expression> const& partiallyParsedExpression = std::shared_ptr<Expression>()
	);
	std::shared_ptr<Expression> parseUnaryExpression(
		std::shared_ptr<Expression> const& partiallyParsedExpression = std::shared_ptr<Expression>()
	);
	std::shared_ptr<Expression> parseLeftHandSideExpression(
		std::shared_ptr<Expression> const& partiallyParsedExpression = std::shared_ptr<Expression>()
	);
	std::shared_ptr<Expression> parsePrimaryExpression();
	std::shared_ptr<Expression> parseLiterial();
	std::shared_ptr<IfStatement> parseIf();
	std::shared_ptr<WhileStatement> parseWhile();
	std::shared_ptr<ForStatement> parseFor();
	std::shared_ptr<DoWhileStatement> parseDoWhile();
	std::shared_ptr<ContinueStatement> parseContinue();
	std::shared_ptr<BreakStatement> parseBreak();
	std::shared_ptr<ExpressionStatement> parseExpressionStatement();

	TokenStream& m_source;
	std::shared_ptr<BaseAST> m_root;
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
// Block = '{' Statement* '}'
// Statement = IfStatement | WhileStatement | ForStatement | Block |
//             ( DoWhileStatement | Continue | Break | Return |
//             SimpleStatement ) ';'

// ExpressionStatement = Expression
// IfStatement = 'if' '(' Expression ')' Statement ( 'else' Statement )?
// WhileStatement = 'while' '(' Expression ')' Statement
// SimpleStatement = VariableDefinition | ExpressionStatement
// ForStatement = 'for' '(' (SimpleStatement)? ';' (Expression)? ';' (ExpressionStatement)? ')' Statement
// DoWhileStatement = 'do' Statement 'while' '(' Expression ')'
// Continue = 'continue'
// Break = 'break'
// Return = 'return' Expression?
// VariableDefinition = TypeName Identifier ('=' Expression)?

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
/// NumberLiteral = ( HexNumber | DecimalNumber )
/// StringLiteral = '"' ([^"\r\n\\] | '\\' .)* '"'
/// Identifier = [a-zA-Z_] [a-zA-Z_0-9]*
/// @}