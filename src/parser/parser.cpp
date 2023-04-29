#include "parser/Parser.h"
#include "common/defs.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <tuple>
#include <vector>


using namespace minisolc;

void Parser::parse() {
	m_root = parseSourceUnit();
	LOG_INFO("Parse Succeeds.");
}

std::shared_ptr<SourceUnit> Parser::parseSourceUnit() {
	try {
		return std::make_shared<SourceUnit>(parseContractDefinition());
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::shared_ptr<ContractDefinition> Parser::parseContractDefinition() {
	std::string name;
	std::vector<std::shared_ptr<BaseAST>> subnodes;
	try {
		expect(Token::Contract);
		expectGet(Token::Identifier, name);
		expect(Token::LBrace);
		/* contract identifier { ... } */
		while (!match(Token::RBrace)) {
			if (peekCur(Token::Function)) {
				/* Function definition. */
				subnodes.push_back(parseFunctionDefinition());
			} else if (peekCur(isType)) {
				/* Variable definition. */
				subnodes.push_back(parseVariableDeclaration(true));
			} else {
				LOG_WARNING("Expect function definition or variable declaration!");
				throw ContractDefinitionParseError(curTokInfo());
				break;
			}
		}
		return std::make_shared<ContractDefinition>(name, std::move(subnodes));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::shared_ptr<VariableDeclaration> Parser::parseVariableDeclaration(bool end) {
	std::string name;
	std::shared_ptr<BaseAST> type;
	std::shared_ptr<BaseAST> expr;

	try {
		type = parseTypeName();
		expectGet(Token::Identifier, name);
		if (match(Token::Assign)) {
			expr = parseExpression();
		}
		if (end)
			expect(Token::Semicolon);

		return std::make_shared<VariableDeclaration>(name, std::move(type), std::move(expr));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::shared_ptr<FunctionDefinition> Parser::parseFunctionDefinition() {
	std::string name;
	std::string state;
	std::string vis;
	std::shared_ptr<ParameterList> paramList;
	std::shared_ptr<TypeName> returnType;
	std::shared_ptr<Block> block;

	StateMutability stateMutability{StateMutability::Nonpayable};
	Visibility visibility{Visibility::Default};

	/* function identifier( parameter-list )
	   ┌─ visibility
	 ──┤
	   └─ state-mutability
	   returns (type)
	   block
	   ;
	*/
	try {
		expect(Token::Function);
		expectGet(Token::Identifier, name);
		paramList = parseParameterList();

		while (true) {
			if (matchGet(isStateMutability, vis)) {
				stateMutability = stateMutabilityByName(vis);
			} else if (matchGet(isVisibility, state)) {
				visibility = visibilityByName(state);
			} else {
				break;
			}
		}

		if (match(Token::Returns)) {
			expect(Token::LParen);
			returnType = parseTypeName();
			expect(Token::RParen);
		}

		block = parseBlock();

		return std::make_shared<FunctionDefinition>(
			name, std::move(paramList), stateMutability, visibility, std::move(returnType), std::move(block));

	} catch (ParseError& e) {
		e.print();
	};
	return nullptr;
}

std::shared_ptr<ParameterList> Parser::parseParameterList() {
	std::vector<std::shared_ptr<BaseAST>> params;
	/* (Type variable, Type variable, ..., Type variable) */
	try {
		expect(Token::LParen);
		if (match(Token::RParen)) {
			return nullptr;
		} else {
			while (true) {
				params.push_back(parseVariableDeclaration(false));
				if (match(Token::Comma)) {
					continue;
				} else {
					expect(Token::RParen);
					break;
				}
			}
		}
		return std::make_shared<ParameterList>(std::move(params));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::shared_ptr<TypeName> Parser::parseTypeName() {
	try {
		std::string type;
		expectGet(isType, type);
		return std::make_shared<ElementaryTypeName>(keywordByName(type));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::shared_ptr<Block> Parser::parseBlock() {
	std::vector<std::shared_ptr<BaseAST>> stmts;
	/* {...} */
	try {
		expect(Token::LBrace);
		while (!match(Token::RBrace)) {
			std::shared_ptr<BaseAST> stmt;
			stmt = parseStatement();
			stmts.push_back(std::move(stmt));
		}
		return std::make_shared<Block>(std::move(stmts));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::shared_ptr<Statement> Parser::parseStatement() {
	std::shared_ptr<BaseAST> child;
	try {
		child = parseReturn();
		expect(Token::Semicolon);

		return std::make_shared<Statement>(std::move(child));
	} catch (ParseError& e) {
		e.print();
	};
	return nullptr;
}

std::shared_ptr<Return> Parser::parseReturn() {
	std::shared_ptr<BaseAST> expr;

	expect(Token::Return);
	expr = parseExpression();
	return std::make_shared<Return>(std::move(expr));
}

std::shared_ptr<Expression> Parser::parseExpression(std::shared_ptr<Expression> const& partiallyParsedExpression) {
	std::shared_ptr<Expression> expr = parseBinaryExpression(4, partiallyParsedExpression);
	Token tok = curTok();
	std::string value;
	
	if (isAssignmentOp(tok))
	{
		match(isAssignmentOp); // advance()
		std::shared_ptr<Expression> rhs = parseExpression();
		return std::make_shared<Assignment>(partiallyParsedExpression, value, rhs);
	}
	return expr;
}

std::shared_ptr<Expression> Parser::parsePrimaryExpression() {
	Token tok = curTok();
	std::string value;
	if (isLiteral(tok)) {
		return parseLiterial();
	} else if (tok == Token::Identifier){
		// Identifier
		matchGet([](Token tok) { return tok == Token::Identifier; }, value);
		return std::make_shared<Identifier>(value);
	} else if (tok == Token::LParen) {
		match(Token::LParen);
		try {
			std::shared_ptr<Expression> expr = parseExpression();
			expect(Token::RParen);
			return expr;
		} catch (ParseError& e) {
			LOG_WARNING("Parse fails.");
			e.print();
		}
	} 
	return nullptr;
}

std::shared_ptr<Expression>
Parser::parseBinaryExpression(
	int minPrecedence, 
	std::shared_ptr<Expression> const& partiallyParsedExpression
) {
	std::shared_ptr<Expression> expr = parseUnaryExpression(partiallyParsedExpression);
	Token tok;
	std::string value;
	for (int precedence = curTokInfo().m_precedence; precedence >= minPrecedence; --precedence) {
		while (curTokInfo().m_precedence == precedence) {
			tok = curTok();
			try {
				expectGet([](Token tok) { return isBinaryOp(tok) || isCompareOp(tok); }, value);
				std::shared_ptr<Expression> rhs = parseBinaryExpression(precedence + 1);
				expr = std::make_shared<BinaryOp>(expr, value, rhs);
			} catch (ParseError& e) {
				LOG_WARNING("Parse fails.");
				e.print();
			}
		}
	}
	return expr;
}

std::shared_ptr<Expression> Parser::parseUnaryExpression(
	std::shared_ptr<Expression> const& partiallyParsedExpression
) {
	Token tok = curTok();
	std::string value;
	if (partiallyParsedExpression == nullptr && isUnaryOp(tok))
	{
		// prefix expression
		matchGet(isUnaryOp, value);
		std::shared_ptr<Expression> subexpr = parseUnaryExpression();
		return std::make_shared<UnaryOp>(value, subexpr, true);
	} else {
		std::shared_ptr<Expression> subexpr = parseLeftHandSideExpression(std::move(partiallyParsedExpression));
		tok = curTok();
		auto isCount = [](Token tok) -> bool { return tok == Token::Inc || tok == Token::Dec; };
		if (!isCount(tok))
		{
			// not postfix expression
			return subexpr;
		}
		// postfix expression
		matchGet(isCount, value);
		return std::make_shared<UnaryOp>(value, subexpr, false);
	}
}

std::shared_ptr<Expression> Parser::parseLeftHandSideExpression(
	std::shared_ptr<Expression> const& partiallyParsedExpression
) {
	std::shared_ptr<Expression> expr;
	if (partiallyParsedExpression == nullptr)
	{
		expr = parsePrimaryExpression();
	} else {
		expr = partiallyParsedExpression;
	}

	Token tok;
	std::string value;
	for(;;)
	{
		tok = curTok();
		switch (tok)
		{
		case Token::LBrack:
		{
			// index range
			LOG_WARNING("Not implemented.");
			// match(Token::LBrack); // advance()
			// std::shared_ptr<Expression> index, endIndex;
			// if (tok != Token::RBrack && tok != Token::Colon)
			// 	index = parseExpression();
			// if (tok == Token::Colon)
			// {
			// 	...
			// } else {
			// 	expect(Token::RBrack);
			// 	...
			// }
			break;
		}
		case Token::Period: /* . */
		{
			LOG_WARNING("Not implemented.");
			break;
		}
		case Token::LParen:
		{
			// function call
			LOG_WARNING("Not implemented.");
			break;
		}
		case Token::LBrace:
		{
			LOG_WARNING("Not implemented.");
		}
		default:
			return expr;
		}
	}
}

std::shared_ptr<Expression> Parser::parseLiterial() {
	Token tok = curTok();
	std::string value;
	std::string unit;

	try {
		expectGet(isLiteral, value);
		switch (tok) {
		case Token::TrueLiteral:
			[[fallthrough]];
		case Token::FalseLiteral:
			return std::make_shared<BooleanLiteral>(value);
		case Token::Number:
			if (matchGet(isNumUnit, unit)) {
				return std::make_shared<NumberLiteral>(value, unit);
			}
			return std::make_shared<NumberLiteral>(value);
		case Token::StringLiteral:
			return std::make_shared<StringLiteral>(value);
		default:
			LOG_ERROR("Expect literal!");
			break;
		}
	} catch (ParseError& e) {
		LOG_WARNING("Parse fails.");
		e.print();
	}
	return nullptr;
}