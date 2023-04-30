#pragma once

#include "common/Defs.h"
#include "lexer/Token.h"
#include <corecrt.h>
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

class Declaration: public BaseAST {
public:
	Declaration(std::string name, std::shared_ptr<BaseAST> type = nullptr): m_name(name), m_type(std::move(type)) {}

protected:
	std::string m_name;
	std::shared_ptr<BaseAST> m_type;
};

class SourceUnit: public BaseAST {
public:
	SourceUnit(std::shared_ptr<BaseAST> subnode): m_subnode(std::move(subnode)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "SourceUnitAST" << RESET << std::endl;
		if (m_subnode) {
			printIndent(depth + 1, mask);
			std::cout << "child:" << std::endl;
			m_subnode->Dump(depth + 2, mask);
		}
	}

private:
	std::shared_ptr<BaseAST> m_subnode;
};

class ContractDefinition: public Declaration {
public:
	ContractDefinition(std::string name, std::vector<std::shared_ptr<BaseAST>> subnodes)
		: Declaration(name), m_subnodes(std::move(subnodes)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ContractDefinitionAST" << RESET << std::endl;

		printIndent(depth + 1, set(mask, depth + 1));
		std::cout << "name: " << m_name << std::endl;

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

class VariableDeclaration: public Declaration {
public:
	VariableDeclaration(std::string name, std::shared_ptr<BaseAST> type, std::shared_ptr<BaseAST> expr)
		: Declaration(name, std::move(type)), m_expr(std::move(expr)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "StateVariableDeclarationAST" << RESET << std::endl;

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
	std::shared_ptr<BaseAST> m_expr; // optional
};

class FunctionDefinition: public Declaration {
public:
	FunctionDefinition(
		std::string name,
		std::shared_ptr<BaseAST> param_list,
		StateMutability state_mut,
		Visibility visibility,
		std::shared_ptr<BaseAST> return_type,
		std::shared_ptr<BaseAST> block)
		: Declaration(name, std::move(return_type)), m_param(std::move(param_list)), m_state(state_mut),
		  m_visibility(visibility), m_block(std::move(block)) {}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "FuncDefAST" << RESET << std::endl;

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "params: " << std::endl;

		m_param->Dump(depth + 2, mask);

		printIndent(depth + 1, mask);
		std::cout << "state mutability: " << stateMutabilityToString(m_state) << std::endl;
		if (m_type) {
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
	std::shared_ptr<BaseAST> m_param;
	StateMutability m_state;
	Visibility m_visibility;
	std::shared_ptr<BaseAST> m_block;
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

class TypeName: public BaseAST {};

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

class Block: public BaseAST {
public:
	Block(std::vector<std::shared_ptr<BaseAST>> stmts): stmts(std::move(stmts)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "BlockAST" << RESET << std::endl;

		if (!stmts.empty()) {
			printIndent(depth + 1, mask);
			std::cout << "childs:" << std::endl;
			mask = set(mask, depth + 2);
			for (const auto& stmt: stmts) {
				if (stmt == stmts.back())
					mask = unset(mask, depth + 2);
				stmt->Dump(depth + 2, mask);
			}
		}
	}

private:
	std::vector<std::shared_ptr<BaseAST>> stmts;
};

class Statement: public BaseAST {
public:
	Statement(std::shared_ptr<BaseAST> child): child(std::move(child)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "StatementAST" << RESET << std::endl;

		printIndent(depth + 1, mask);
		std::cout << "child: " << std::endl;

		child->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<BaseAST> child;
};

class Return: public BaseAST {
public:
	Return(std::shared_ptr<BaseAST> expr): expr(std::move(expr)) {}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ReturnAST" << RESET << std::endl;
		if (expr) {
			printIndent(depth + 1, mask);
			std::cout << "expr: " << std::endl;

			expr->Dump(depth + 2, mask);
		}
	}

private:
	std::shared_ptr<BaseAST> expr; // optional
};

class Expression: public BaseAST {};

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

		if (m_unit != "")
			mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "value: " << m_value << std::endl;

		if (m_unit != "") {
			mask = unset(mask, depth + 1);
			printIndent(depth + 1, mask);
			std::cout << "unit: " << m_unit << std::endl;
		}
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

}