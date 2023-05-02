#pragma once

#include "common/Defs.h"
#include "lexer/Token.h"
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>


namespace minisolc {

// 所有 AST 的基类
class BaseAST {
public:
	virtual ~BaseAST() = default;

	virtual void Dump(size_t depth, size_t mask) const {};

protected:
	void printIndent(size_t depth, size_t mask) const {
		for (size_t i = 1; i <= depth; ++i) {
			if (i == depth && (mask & (1 << i))) {
				std::cout << "├─";
			} else if (i == depth) {
				std::cout << "└─";
			} else if (mask & (1 << i)) {
				std::cout << "│ ";
			} else {
				std::cout << "  ";
			}
		}
	};

	std::string astColor(size_t depth) const {
		std::vector<std::string> colors = {GREEN, YELLOW, BLUE, MAGENTA, CYAN};
		return colors[depth % colors.size()];
	}

	size_t set(size_t mask, size_t pos) const { return mask | (1 << pos); }

	size_t unset(size_t mask, size_t pos) const { return mask & ~(1 << pos); }
};

class Statement: public BaseAST {};
class SimpleStatement: public Statement {};
class Expression: public BaseAST {};
class TypeName: public BaseAST {};
class Declaration {
public:
	Declaration(std::string name, std::shared_ptr<TypeName> type = nullptr): m_name(name), m_type(std::move(type)) {}

protected:
	std::string m_name;
	std::shared_ptr<TypeName> m_type;
};

class SourceUnit: public BaseAST {
public:
	SourceUnit(std::vector<std::shared_ptr<BaseAST>> subnodes): m_subnodes(std::move(subnodes)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "SourceUnitAST" << RESET << std::endl;

		if (!m_subnodes.empty()) {
			printIndent(depth + 1, mask);
			std::cout << "childs:" << std::endl;
			mask = set(mask, depth + 2);
			for (const auto& subnode: m_subnodes) {
				if (subnode == m_subnodes.back())
					mask = unset(mask, depth + 2);
				subnode->Dump(depth + 2, mask);
			}
		}
	}

private:
	std::vector<std::shared_ptr<BaseAST>> m_subnodes;
};

class VariableDefinition: public Declaration, public SimpleStatement {
public:
	VariableDefinition(std::string name, std::shared_ptr<TypeName> type, std::shared_ptr<Expression> expr)
		: Declaration(name, std::move(type)), m_expr(std::move(expr)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "VariableDefinitionAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "name: " << m_name << std::endl;

		if (!m_expr)
			mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "type: " << std::endl;

		m_type->Dump(depth + 2, mask);

		if (m_expr) {
			mask = unset(mask, depth + 1);
			printIndent(depth + 1, mask);
			std::cout << "expr: " << std::endl;

			m_expr->Dump(depth + 2, mask);
		}
	}

private:
	std::shared_ptr<Expression> m_expr; // optional
};

class ParameterList: public BaseAST {
public:
	ParameterList(std::vector<std::shared_ptr<BaseAST>> params): params(std::move(params)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ParameterListAST" << RESET << std::endl;

		if (!params.empty()) {
			printIndent(depth + 1, mask);
			std::cout << "params:" << std::endl;
			mask = set(mask, depth + 2);
			for (const auto& param: params) {
				if (param == params.back())
					mask = unset(mask, depth + 2);
				param->Dump(depth + 2, mask);
			}
		}
	}

private:
	std::vector<std::shared_ptr<BaseAST>> params; // type, ident
};

class Block: public Statement {
public:
	Block(std::vector<std::shared_ptr<Statement>> stmts): m_stmts(std::move(stmts)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "BlockAST" << RESET << std::endl;

		if (!m_stmts.empty()) {
			printIndent(depth + 1, mask);
			std::cout << "childs:" << std::endl;
			mask = set(mask, depth + 2);
			for (const auto& stmt: m_stmts) {
				if (stmt == m_stmts.back())
					mask = unset(mask, depth + 2);
				stmt->Dump(depth + 2, mask);
			}
		}
	}

private:
	std::vector<std::shared_ptr<Statement>> m_stmts;
};

class FunctionDefinition: public Declaration, public BaseAST {
public:
	FunctionDefinition(
		std::string name,
		std::shared_ptr<ParameterList> param_list,
		Visibility visibility,
		std::shared_ptr<TypeName> return_type,
		std::shared_ptr<Block> block)
		: Declaration(name, std::move(return_type)), m_param(std::move(param_list)), m_visibility(visibility),
		  m_block(std::move(block)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "FuncDefAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		if (m_param) {
			printIndent(depth + 1, mask);
			std::cout << "params: " << std::endl;
			m_param->Dump(depth + 2, mask);
		}

		if (m_type) {
			printIndent(depth + 1, mask);
			std::cout << "return type:" << std::endl;
			m_type->Dump(depth + 2, mask);
		}

		printIndent(depth + 1, mask);
		std::cout << "visibility: " << visibilityToString(m_visibility) << std::endl;

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "block:" << std::endl;

		m_block->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<ParameterList> m_param;
	Visibility m_visibility;
	std::shared_ptr<Block> m_block;
};


class ElementaryTypeName: public TypeName {
public:
	ElementaryTypeName(Token type): type(type) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ElementaryTypeNameAST" << RESET << std::endl;

		printIndent(depth + 1, mask);
		std::cout << "type: " << tokenToString(type) << std::endl;
	}

private:
	Token type;
};


class ReturnStatement: public Statement {
public:
	ReturnStatement(std::shared_ptr<Expression> expr): m_expr(std::move(expr)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ReturnAST" << RESET << std::endl;
		if (m_expr) {
			printIndent(depth + 1, mask);
			std::cout << "expr: " << std::endl;

			m_expr->Dump(depth + 2, mask);
		}
	}

private:
	std::shared_ptr<Expression> m_expr; // optional
};

class PrimaryExpression: public Expression {
public:
	PrimaryExpression(std::string value): m_value(value) {}

protected:
	std::string m_value;
};

class Identifier: public PrimaryExpression {
public:
	Identifier(std::string value): PrimaryExpression(value) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "IdentifierAST" << RESET << std::endl;

		printIndent(depth + 1, mask);
		std::cout << "value: " << m_value << std::endl;
	}
};

class BooleanLiteral: public PrimaryExpression {
public:
	BooleanLiteral(std::string value): PrimaryExpression(value) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "BooleanLiteralAST" << RESET << std::endl;

		printIndent(depth + 1, mask);
		std::cout << "value: " << m_value << std::endl;
	}
};

class StringLiteral: public PrimaryExpression {
public:
	StringLiteral(std::string value): PrimaryExpression(value) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "StringLiteralAST" << RESET << std::endl;

		printIndent(depth + 1, mask);
		std::cout << "value: " << m_value << std::endl;
	}
};

class NumberLiteral: public PrimaryExpression {
public:
	NumberLiteral(std::string value, std::string unit = ""): PrimaryExpression(value), m_unit(unit) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "NumberLiteralAST" << RESET << std::endl;

		printIndent(depth + 1, mask);
		std::cout << "value: " << m_value << std::endl;
	}

private:
	std::string m_unit;
};

class Assignment: public Expression {
public:
	Assignment(std::shared_ptr<Expression> lhs, std::string assignOp, std::shared_ptr<Expression> rhs)
		: m_leftHandSide(std::move(lhs)), m_assigmentOp(std::move(assignOp)), m_rightHandSide(std::move(rhs)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "AssignmentAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "leftHandSide: " << std::endl;

		m_leftHandSide->Dump(depth + 2, mask);

		printIndent(depth + 1, mask);
		std::cout << "assigmentOp: " << m_assigmentOp << std::endl;

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "rightHandSide: " << std::endl;

		m_rightHandSide->Dump(depth + 2, mask);
	}


private:
	std::shared_ptr<Expression> m_leftHandSide;
	std::string m_assigmentOp;
	std::shared_ptr<Expression> m_rightHandSide;
};

class BinaryOp: public Expression {
public:
	BinaryOp(std::shared_ptr<Expression> lhs, std::string binaryOp, std::shared_ptr<Expression> rhs)
		: m_leftHandSide(std::move(lhs)), m_binaryOp(std::move(binaryOp)), m_rightHandSide(std::move(rhs)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "BinaryOpAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "leftHandSide: " << std::endl;

		m_leftHandSide->Dump(depth + 2, mask);

		printIndent(depth + 1, mask);
		std::cout << "binaryOp: " << m_binaryOp << std::endl;

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "rightHandSide: " << std::endl;

		m_rightHandSide->Dump(depth + 2, mask);
	}


private:
	std::shared_ptr<Expression> m_leftHandSide;
	std::string m_binaryOp;
	std::shared_ptr<Expression> m_rightHandSide;
};

class UnaryOp: public Expression {
public:
	UnaryOp(std::string unaryOp, std::shared_ptr<Expression> subExpr, bool isPrefix)
		: m_unaryOp(std::move(unaryOp)), m_subExpr(std::move(subExpr)), m_isPrefix(isPrefix) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "UnaryOpAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "unaryOp: " << (m_isPrefix ? "prefix " : "postfix ") << m_unaryOp << std::endl;

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "subExpr: " << std::endl;

		m_subExpr->Dump(depth + 2, mask);
	}

private:
	std::string m_unaryOp;
	std::shared_ptr<Expression> m_subExpr;
	bool m_isPrefix;
};

class IfStatement: public Statement {
public:
	IfStatement(
		std::shared_ptr<Expression> condition,
		std::shared_ptr<Statement> thenStatement,
		std::shared_ptr<Statement> elseStatement = nullptr)
		: m_condition(std::move(condition)), m_thenStatement(std::move(thenStatement)),
		  m_elseStatement(std::move(elseStatement)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "IfStatementAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "condition: " << std::endl;

		m_condition->Dump(depth + 2, mask);

		if (!m_thenStatement) {
			mask = unset(mask, depth + 1);
		}
		printIndent(depth + 1, mask);
		std::cout << "thenStatement: " << std::endl;

		m_thenStatement->Dump(depth + 2, mask);

		if (m_elseStatement) {
			mask = unset(mask, depth + 1);
			printIndent(depth + 1, mask);
			std::cout << "elseStatement: " << std::endl;

			m_elseStatement->Dump(depth + 2, mask);
		}
	}

private:
	std::shared_ptr<Expression> m_condition;
	std::shared_ptr<Statement> m_thenStatement;
	std::shared_ptr<Statement> m_elseStatement;
};

class WhileStatement: public Statement {
public:
	WhileStatement(std::shared_ptr<Expression> condition, std::shared_ptr<Statement> body)
		: m_condition(std::move(condition)), m_body(std::move(body)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "WhileStatementAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "condition: " << std::endl;

		m_condition->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "body: " << std::endl;

		m_body->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<Expression> m_condition;
	std::shared_ptr<Statement> m_body;
};

class ForStatement: public Statement {
public:
	ForStatement(
		std::shared_ptr<SimpleStatement> init,
		std::shared_ptr<Expression> condition,
		std::shared_ptr<Expression> update,
		std::shared_ptr<Statement> body)
		: m_init(std::move(init)), m_condition(std::move(condition)), m_update(std::move(update)),
		  m_body(std::move(body)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ForStatementAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "init: " << std::endl;

		m_init->Dump(depth + 2, mask);

		printIndent(depth + 1, mask);
		std::cout << "condition: " << std::endl;

		m_condition->Dump(depth + 2, mask);

		printIndent(depth + 1, mask);
		std::cout << "update: " << std::endl;

		m_update->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "body: " << std::endl;

		m_body->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<SimpleStatement> m_init;
	std::shared_ptr<Expression> m_condition;
	std::shared_ptr<Expression> m_update;
	std::shared_ptr<Statement> m_body;
};

class DoWhileStatement: public Statement {
public:
	DoWhileStatement(std::shared_ptr<Expression> condition, std::shared_ptr<Statement> body)
		: m_body(std::move(body)), m_condition(std::move(condition)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "DoWhileStatementAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "body: " << std::endl;

		m_body->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "condition: " << std::endl;

		m_condition->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<Statement> m_body;
	std::shared_ptr<Expression> m_condition;
};

class BreakStatement: public Statement {
public:
	BreakStatement() = default;

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "BreakStatementAST" << RESET << std::endl;
	}
};

class ContinueStatement: public Statement {
public:
	ContinueStatement() = default;

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ContinueStatementAST" << RESET << std::endl;
	}
};

class ExpressionStatement: public SimpleStatement {
public:
	ExpressionStatement(std::shared_ptr<Expression> expr): m_expr(std::move(expr)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ExpressionStatementAST" << RESET << std::endl;

		printIndent(depth + 1, mask);
		std::cout << "expr: " << std::endl;

		m_expr->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<Expression> m_expr;
};

class IndexAccess: public Expression {
public:
	IndexAccess(std::shared_ptr<Expression> expr, std::shared_ptr<Expression> index)
		: m_expr(std::move(expr)), m_index(std::move(index)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "IndexAccessAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "expr: " << std::endl;

		m_expr->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "index: " << std::endl;

		m_index->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<Expression> m_expr;
	std::shared_ptr<Expression> m_index;
};

class FunctionCall: public Expression {
public:
	FunctionCall(std::shared_ptr<Expression> expr, std::vector<std::shared_ptr<Expression>> args)
		: m_expr(std::move(expr)), m_args(std::move(args)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "FunctionCallAST" << RESET << std::endl;

		if (!m_args.empty())
			mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "expr: " << std::endl;

		m_expr->Dump(depth + 2, mask);

		if (!m_args.empty()) {
			mask = unset(mask, depth + 1);
			printIndent(depth + 1, mask);
			std::cout << "args: " << std::endl;
			mask = set(mask, depth + 2);
			for (const auto& arg: m_args) {
				if (arg == m_args.back())
					mask = unset(mask, depth + 2);
				arg->Dump(depth + 2, mask);
			}
		}
	}

private:
	std::shared_ptr<Expression> m_expr;
	std::vector<std::shared_ptr<Expression>> m_args;
};

class MemberAccess: public Expression {
public:
	MemberAccess(std::shared_ptr<Expression> expr, std::string member)
		: m_expr(std::move(expr)), m_member(std::move(member)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "MemberAccessAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "expr: " << std::endl;

		m_expr->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "member: " << m_member << std::endl;
	}

private:
	std::shared_ptr<Expression> m_expr;
	std::string m_member;
};


}