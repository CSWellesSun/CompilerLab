#include "parser/Parser.h"
#include "common/defs.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <tuple>
#include <vector>


using namespace minisolc;

void Parser::parse() { m_root = parseSourceUnit(); }

std::unique_ptr<SourceUnit> Parser::parseSourceUnit() {
	try {
		return std::make_unique<SourceUnit>(parseContractDefinition());
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::unique_ptr<ContractDefinition> Parser::parseContractDefinition() {
	std::string name;
	std::vector<std::unique_ptr<BaseAST>> subnodes;
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
		return std::make_unique<ContractDefinition>(name, std::move(subnodes));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(bool end) {
	std::string name;
	std::unique_ptr<BaseAST> type;
	std::unique_ptr<BaseAST> expr;

	try {
		type = parseTypeName();
		expectGet(Token::Identifier, name);
		if (match(Token::Assign)) {
			expr = parseExpression();
		}
		if (end)
			expect(Token::Semicolon);

		return std::make_unique<VariableDeclaration>(name, std::move(type), std::move(expr));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::unique_ptr<FunctionDefinition> Parser::parseFunctionDefinition() {
	std::string name;
	std::string state;
	std::string vis;
	std::unique_ptr<ParameterList> paramList;
	std::unique_ptr<TypeName> returnType;
	std::unique_ptr<Block> block;

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

		return std::make_unique<FunctionDefinition>(
			name, std::move(paramList), stateMutability, visibility, std::move(returnType), std::move(block));

	} catch (ParseError& e) {
		e.print();
	};
	return nullptr;
}

std::unique_ptr<ParameterList> Parser::parseParameterList() {
	std::vector<std::unique_ptr<BaseAST>> params;
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
		return std::make_unique<ParameterList>(std::move(params));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::unique_ptr<TypeName> Parser::parseTypeName() {
	try {
		std::string type;
		expectGet(isType, type);
		return std::make_unique<ElementaryTypeName>(keywordByName(type));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::unique_ptr<Block> Parser::parseBlock() {
	std::vector<std::unique_ptr<BaseAST>> stmts;
	/* {...} */
	try {
		expect(Token::LBrace);
		while (!match(Token::RBrace)) {
			std::unique_ptr<BaseAST> stmt;
			stmt = parseStatement();
			stmts.push_back(std::move(stmt));
		}
		return std::make_unique<Block>(std::move(stmts));
	} catch (ParseError& e) {
		e.print();
	}
	return nullptr;
}

std::unique_ptr<Statement> Parser::parseStatement() {
	std::unique_ptr<BaseAST> child;
	try {
		child = parseReturn();
		expect(Token::Semicolon);

		return std::make_unique<Statement>(std::move(child));
	} catch (ParseError& e) {
		e.print();
	};
	return nullptr;
}

std::unique_ptr<Return> Parser::parseReturn() {
	std::unique_ptr<BaseAST> expr;

	expect(Token::Return);
	expr = parseExpression();
	return std::make_unique<Return>(std::move(expr));
}

std::unique_ptr<Expression> Parser::parseExpression() { return parsePrimaryExpression(); }

std::unique_ptr<PrimaryExpression> Parser::parsePrimaryExpression() {
	Token tok = curTok();
	std::string value;
	std::string unit;

	try {
		expectGet(isLiteral, value);

		switch (tok) {
		case Token::TrueLiteral:
			[[fallthrough]];
		case Token::FalseLiteral:
			return std::make_unique<BooleanLiteral>(value);
		case Token::Number:
			if (matchGet(isNumUnit, unit)) {
				return std::make_unique<NumberLiteral>(value, unit);
			}
			return std::make_unique<NumberLiteral>(value);
		case Token::StringLiteral:
			return std::make_unique<StringLiteral>(value);
		default:
			LOG_ERROR("Expect literal!");
			break;
		}
	} catch (ParseError& e) {
<<<<<<< HEAD
		std::cout << "helloError" << '\n';
=======
>>>>>>> c45277eeed906a3d0a3c8940b1cb8c900a0cf0be
		e.print();
	}
	return nullptr;
}