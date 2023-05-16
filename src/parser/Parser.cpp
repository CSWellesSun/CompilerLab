#include "parser/Parser.h"
#include "common/Defs.h"
#include "lexer/Token.h" // for precedence()
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
	std::vector<std::shared_ptr<BaseAST>> subnodes;
	try {
		while (!match(Token::EOS)) {
			if (peekCur(Token::Function)) {
				/* Function definition. */
				subnodes.push_back(parseFunctionDefinition());
			} else if (peekCur(isType)) {
				/* Variable definition. */
				subnodes.push_back(parseVariableDefinition());
				expect(Token::Semicolon);
			} else if (peekCur(Token::Struct)) {
				subnodes.push_back(parseStructDefinition());
				expect(Token::Semicolon);
			} else {
				LOG_WARNING("Expect function definition or variable declaration!");
				throw ContractDefinitionParseError(curTokInfo());
				break;
			}
		}
		return std::make_shared<SourceUnit>(std::move(subnodes));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::shared_ptr<VariableDefinition> Parser::parseVariableDefinition() {
	std::string name;
	std::shared_ptr<TypeName> type;
	std::shared_ptr<Expression> expr;

	try {
		type = parseTypeName();
		expectGet(Token::Identifier, name);
		if (match(Token::Assign)) {
			expr = parseExpression();
		} else if (match(Token::LBrack)) {
			/* Array */
			expr = parseLiterial(); // array of size 0 is not allowed.
			if (expr->GetASTType() != ElementASTTypes::NumberLiteral) {
				LOG_WARNING("Parse Array Fails!");
				throw ParseError(curTokInfo());
			}
			expect(Token::RBrack);

			if (match(Token::Assign)) {
				/* Initialize list. */
				LOG_WARNING("Not implemented.");
			}
			return std::make_shared<ArrayDefinition>(name, std::move(type), std::move(expr));
		}
		/* Plain variable definition. */
		return std::make_shared<PlainVariableDefinition>(name, std::move(type), std::move(expr));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::shared_ptr<StructDefinition> Parser::parseStructDefinition() {
	// Partially completes.
	std::string name;
	try {
		expect(Token::Struct);
		std::vector<std::shared_ptr<VariableDefinition>> struct_members;
		std::shared_ptr<TypeName> type = std::make_shared<ElementaryTypeName>(Token::Struct);
		expectGet(Token::Identifier, name);

		if (match(Token::LBrace)) {
			while (!match(Token::RBrace)) {
				struct_members.push_back(parseVariableDefinition());
				match(Token::Semicolon);
			}
		}

		return std::make_shared<StructDefinition>(name, std::move(struct_members));
	} catch (ParseError& e) {
		e.print();
	}
	/* TODO: It may be necessary for the parser to store all defined structs
	   so as to define structural variables afterwards.*/
	return nullptr;
}

std::shared_ptr<FunctionDefinition> Parser::parseFunctionDefinition() {
	std::string name;
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

		if (matchGet(isVisibility, vis)) {
			visibility = visibilityByName(vis);
		}

		if (match(Token::Returns)) {
			expect(Token::LParen);
			returnType = parseTypeName();
			expect(Token::RParen);
		}

		block = parseBlock();

		return std::make_shared<
			FunctionDefinition>(name, std::move(paramList), visibility, std::move(returnType), std::move(block));

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
				/* Here struct parameter is not implemented. */
				params.push_back(parseVariableDefinition());
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
	std::vector<std::shared_ptr<Statement>> stmts;
	/* {...} */
	try {
		expect(Token::LBrace);
		while (!match(Token::RBrace)) {
			std::shared_ptr<Statement> stmt;
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
	std::shared_ptr<Statement> stmt;
	try {
		if (peekCur(Token::Return)) {
			stmt = parseReturn();
			expect(Token::Semicolon);
		} else if (peekCur(Token::If))
			stmt = parseIf();
		else if (peekCur(Token::While))
			stmt = parseWhile();
		else if (peekCur(Token::For))
			stmt = parseFor();
		else if (peekCur(Token::Do)) {
			stmt = parseDoWhile();
			expect(Token::Semicolon);
		} else if (peekCur(Token::Continue)) {
			stmt = parseContinue();
			expect(Token::Semicolon);
		} else if (peekCur(Token::Break)) {
			stmt = parseBreak();
			expect(Token::Semicolon);
		} else if (peekCur(Token::Semicolon))
			stmt = nullptr;
		else if (peekCur(isType)) {
			stmt = parseVariableDefinition();
			expect(Token::Semicolon);
		} else if (peekCur(Token::Struct)) {
			stmt = parseStructDefinition();
			expect(Token::Semicolon);
		} else if (peekCur(Token::LBrace))
			stmt = parseBlock();
		else {
			stmt = parseExpressionStatement();
			expect(Token::Semicolon);
		}

		return stmt;
	} catch (ParseError& e) {
		e.print();
	};
	return nullptr;
}

std::shared_ptr<ReturnStatement> Parser::parseReturn() {
	std::shared_ptr<Expression> expr;

	expect(Token::Return);
	expr = parseExpression();
	return std::make_shared<ReturnStatement>(std::move(expr));
}

std::shared_ptr<Expression> Parser::parseExpression(std::shared_ptr<Expression> const& partiallyParsedExpression) {
	std::shared_ptr<Expression> expr = parseBinaryExpression(4, partiallyParsedExpression);
	Token tok = curTok();
	std::string value;

	if (isAssignmentOp(tok)) {
		// Assignment.
		value = curVal();
		advance(); // eat assignment operator
		std::shared_ptr<Expression> rhs = parseExpression();
		return std::make_shared<Assignment>(expr, value, rhs);
	}
	return expr;
}

std::shared_ptr<Expression> Parser::parsePrimaryExpression() {
	Token tok = curTok();
	std::string value;
	if (isLiteral(tok)) {
		// literals
		return parseLiterial();
	} else if (tok == Token::Identifier) {
		// identifier
		value = curVal();
		advance(); // eat;
		return std::make_shared<Identifier>(value);
	} else if (tok == Token::LParen) {
		// handle parentheses in expressions
		// e.g. a = (b + c) * d
		match(Token::LParen);
		try {
			std::shared_ptr<Expression> expr = parseExpression();
			expect(Token::RParen);
			return expr;
		} catch (ParseError& e) {
			LOG_WARNING("Parse fails.");
			e.print();
		}
	} else {
		LOG_WARNING("Invalid character %s in parse primary expression.", curVal().c_str());
	}
	return nullptr;
}

std::shared_ptr<Expression>
Parser::parseBinaryExpression(int minPrecedence, std::shared_ptr<Expression> const& partiallyParsedExpression) {
	std::shared_ptr<Expression> expr = parseUnaryExpression(partiallyParsedExpression);
	Token tok;
	std::string value;
	for (int curPrecedence = precedence(curTok()); curPrecedence >= minPrecedence; --curPrecedence) {
		while (precedence(curTok()) == curPrecedence) {
			tok = curTok();
			try {
				// parse binary operation
				expectGet([](Token tok) { return isBinaryOp(tok) || isCompareOp(tok); }, value);
				std::shared_ptr<Expression> rhs = parseBinaryExpression(curPrecedence + 1);
				expr = std::make_shared<BinaryOp>(expr, value, rhs);
			} catch (ParseError& e) {
				LOG_WARNING("Parse fails.");
				e.print();
			}
		}
	}
	return expr;
}

std::shared_ptr<Expression> Parser::parseUnaryExpression(std::shared_ptr<Expression> const& partiallyParsedExpression) {
	Token tok = curTok();
	std::string value;
	if (partiallyParsedExpression == nullptr && isUnaryOp(tok)) {
		// prefix expression
		matchGet(isUnaryOp, value);
		std::shared_ptr<Expression> subexpr = parseUnaryExpression();
		return std::make_shared<UnaryOp>(value, subexpr, true);
	} else {
		std::shared_ptr<Expression> subexpr = parseLeftHandSideExpression(partiallyParsedExpression);
		tok = curTok();
		auto isCount = [](Token tok) -> bool { return tok == Token::Inc || tok == Token::Dec; };
		if (!isCount(tok)) {
			// not postfix expression
			return subexpr;
		}
		// postfix expression
		matchGet(isCount, value);
		return std::make_shared<UnaryOp>(value, subexpr, false);
	}
}

std::shared_ptr<Expression>
Parser::parseLeftHandSideExpression(std::shared_ptr<Expression> const& partiallyParsedExpression) {
	std::shared_ptr<Expression> expr;
	if (partiallyParsedExpression == nullptr) {
		expr = parsePrimaryExpression();
	} else {
		expr = partiallyParsedExpression;
	}

	Token tok;
	std::string value;
	for (;;) {
		tok = curTok();
		switch (tok) {
		case Token::LBrack: {
			/* Index range. */
			advance();
			std::shared_ptr<Expression> index = parseExpression();
			expect(Token::RBrack);
			expr = std::make_shared<IndexAccess>(expr, index);
			break;
		}
		case Token::Period: /* . */
		{
			/* Access structure members. */
			advance();
			matchGet(Token::Identifier, value);
			expr = std::make_shared<MemberAccess>(expr, value);
			break;
		}
		case Token::LParen: {
			/* Function call. */
			advance();
			std::vector<std::shared_ptr<Expression>> args;
			if (curTok() != Token::RParen) {
				args.push_back(parseExpression());
				while (curTok() == Token::Comma) {
					advance();
					args.push_back(parseExpression());
				}
			}
			expect(Token::RParen);
			expr = std::make_shared<FunctionCall>(expr, args);
			break;
		}
		// case Token::LBrace: {
		// 	LOG_WARNING("Not implemented.");
		// }
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
			/* Boolean literal. */
			return std::make_shared<BooleanLiteral>(value);
		case Token::Number:
			/* Number literal. */
			return std::make_shared<NumberLiteral>(value);
		case Token::StringLiteral:
			/* String literal. */
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

std::shared_ptr<IfStatement> Parser::parseIf() {
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> thenStatement;
	std::shared_ptr<Statement> elseStatement;

	try {
		expect(Token::If);
		expect(Token::LParen);
		condition = parseExpression();
		expect(Token::RParen);
		thenStatement = parseStatement();
		if (curTok() == Token::Else) {
			advance();
			elseStatement = parseStatement();
		}
	} catch (ParseError& e) {
		LOG_WARNING("Parse fails.");
		e.print();
	}
	return std::make_shared<IfStatement>(condition, thenStatement, elseStatement);
}

std::shared_ptr<WhileStatement> Parser::parseWhile() {
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> body;

	try {
		expect(Token::While);
		expect(Token::LParen);
		condition = parseExpression();
		expect(Token::RParen);
		body = parseStatement();
	} catch (ParseError& e) {
		LOG_WARNING("Parse fails.");
		e.print();
	}
	return std::make_shared<WhileStatement>(condition, body);
}

std::shared_ptr<ForStatement> Parser::parseFor() {
	std::shared_ptr<SimpleStatement> init;
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Expression> step;
	std::shared_ptr<Statement> body;

	try {
		expect(Token::For);
		expect(Token::LParen);
		if (peekCur(isType) || peekCur(Token::Struct))
			init = parseVariableDefinition();
		else
			init = parseExpressionStatement();
		expect(Token::Semicolon);
		condition = parseExpression();
		expect(Token::Semicolon);
		step = parseExpression();
		expect(Token::RParen);
		body = parseStatement();
	} catch (ParseError& e) {
		LOG_WARNING("Parse fails.");
		e.print();
	}
	return std::make_shared<ForStatement>(init, condition, step, body);
}
std::shared_ptr<DoWhileStatement> Parser::parseDoWhile() {
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Statement> body;

	try {
		expect(Token::Do);
		body = parseStatement();
		expect(Token::While);
		expect(Token::LParen);
		condition = parseExpression();
		expect(Token::RParen);
	} catch (ParseError& e) {
		LOG_WARNING("Parse fails.");
		e.print();
	}
	return std::make_shared<DoWhileStatement>(condition, body);
}
std::shared_ptr<ContinueStatement> Parser::parseContinue() {
	try {
		expect(Token::Continue);
	} catch (ParseError& e) {
		LOG_WARNING("Parse fails.");
		e.print();
	}
	return std::make_shared<ContinueStatement>();
}
std::shared_ptr<BreakStatement> Parser::parseBreak() {
	try {
		expect(Token::Break);
	} catch (ParseError& e) {
		LOG_WARNING("Parse fails.");
		e.print();
	}
	return std::make_shared<BreakStatement>();
}

std::shared_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
	std::shared_ptr<Expression> expr;
	try {
		expr = parseExpression();
	} catch (ParseError& e) {
		LOG_WARNING("Parse fails.");
		e.print();
	}
	return std::make_shared<ExpressionStatement>(expr);
}