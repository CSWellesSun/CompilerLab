#pragma once

#include "lexer/Token.h"
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>


namespace minisolc {

// 所有 AST 的基类
class BaseAST {
public:
	virtual ~BaseAST() = default;

	virtual void Dump() const {};
};

class Declaration: public BaseAST {
public:
	Declaration(std::string name, std::shared_ptr<BaseAST> type = nullptr): m_name(name), m_type(std::move(type)) {}

protected:
	std::string m_name;
	std::shared_ptr<BaseAST> m_type;
};

// SourceUnit 是 BaseAST
class SourceUnit: public BaseAST {
public:
	SourceUnit(std::shared_ptr<BaseAST> subnode): m_subnode(std::move(subnode)) {}
	void Dump() const override {
		std::cout << "SourceUnitAST { ";
		if (m_subnode)
			m_subnode->Dump();
		std::cout << " }";
	}

private:
	// 用智能指针管理对象
	std::shared_ptr<BaseAST> m_subnode;
};

class ContractDefinition: public Declaration {
public:
	// 用智能指针管理对象
	ContractDefinition(std::string name, std::vector<std::shared_ptr<BaseAST>> subnodes)
		: Declaration(name), m_subnodes(std::move(subnodes)) {}
	void Dump() const override {
		std::cout << "ContractDefinitionAST { ";
		std::cout << m_name << ", ";
		for (const auto& subnode: m_subnodes) {
			subnode->Dump();
			std::cout << ", ";
		}
		std::cout << " }";
	}

private:
	std::vector<std::shared_ptr<BaseAST>> m_subnodes;
};

class VariableDeclaration: public Declaration {
public:
	VariableDeclaration(std::string name, std::shared_ptr<BaseAST> type, std::shared_ptr<BaseAST> expr)
		: Declaration(name, std::move(type)), m_expr(std::move(expr)) {}
	void Dump() const override {
		std::cout << "StateVariableDeclarationAST { ";
		m_type->Dump();
		std::cout << ", " << m_name << ", ";
		if (m_expr)
			m_expr->Dump();
		std::cout << " }";
	}

private:
	std::shared_ptr<BaseAST> m_expr; // optional
};

// FuncDef 也是 BaseAST
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

	void Dump() const override {
		std::cout << "FuncDefAST { "
				  << m_name << ", ";
		m_param->Dump();
		std::cout << ' ' << stateMutabilityToString(m_state) << ", ";
		if (m_type)
			m_type->Dump();
		std::cout << ' ';
		m_block->Dump();
		std::cout << " }";
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
	void Dump() const override {
		std::cout << "ParameterListAST { ";
		for (const auto& param: params) {
			param->Dump();
			std::cout << ", ";
		}
		std::cout << " }";
	}

private:
	std::vector<std::shared_ptr<BaseAST>> params; // type, ident
};

class TypeName: public BaseAST {};

class ElementaryTypeName: public TypeName {
public:
	ElementaryTypeName(Token type): type(type) {}
	void Dump() const override { std::cout << "ElementaryTypeNameAST { " << tokenToString(type) << " }"; }

private:
	Token type;
};

class Block: public BaseAST {
public:
	Block(std::vector<std::shared_ptr<BaseAST>> stmts): stmts(std::move(stmts)) {}
	void Dump() const override {
		std::cout << "BlockAST { ";
		for (const auto& stmt: stmts) {
			stmt->Dump();
			std::cout << ", ";
		}
		std::cout << " }";
	}

private:
	std::vector<std::shared_ptr<BaseAST>> stmts;
};

class Statement: public BaseAST {
public:
	Statement(std::shared_ptr<BaseAST> child): child(std::move(child)) {}
	void Dump() const override {
		std::cout << "StatementAST { ";
		child->Dump();
		std::cout << " }";
	}

private:
	std::shared_ptr<BaseAST> child;
};

class Return: public BaseAST {
public:
	Return(std::shared_ptr<BaseAST> expr): expr(std::move(expr)) {}
	void Dump() const override {
		std::cout << "ReturnAST { ";
		if (expr)
			expr->Dump();
		std::cout << " }";
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
	void Dump() const override { std::cout << "IdentifierAST { " << m_value << " }"; }
};

class BooleanLiteral: public PrimaryExpression {
public:
	BooleanLiteral(std::string value): PrimaryExpression(value) {}
	void Dump() const override { std::cout << "BooleanLiteralAST { " << m_value << " }"; }
};

class StringLiteral: public PrimaryExpression {
public:
	StringLiteral(std::string value): PrimaryExpression(value) {}
	void Dump() const override { std::cout << "StringLiteralAST { " << m_value << " }"; }
};

class NumberLiteral: public PrimaryExpression {
public:
	NumberLiteral(std::string value, std::string unit = ""): PrimaryExpression(value), m_unit(unit) {}
	void Dump() const override { std::cout << "NumberLiteralAST { " << m_value << " }"; }

private:
	std::string m_unit;
};

class Assignment: public Expression {
public:
	Assignment(std::shared_ptr<Expression> lhs, std::string assignOp, std::shared_ptr<Expression> rhs)
		: m_leftHandSide(std::move(lhs)), m_assigmentOp(std::move(assignOp)), m_rightHandSide(std::move(rhs)) {}

	void Dump() const override {
		std::cout << "AssignmentAST { ";
		m_leftHandSide->Dump();
		std::cout << ", " << m_assigmentOp << ", ";
		m_rightHandSide->Dump();
		std::cout << " }";
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

	void Dump() const override {
		std::cout << "BinaryOpAST { ";
		m_leftHandSide->Dump();
		std::cout << ", " << m_binaryOp << ", ";
		m_rightHandSide->Dump();
		std::cout << " }";
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
	void Dump() const override {
		std::cout << "UnaryOpAST { "
				  << (m_isPrefix ? "prefix " : "postfix ")
				  << m_unaryOp << ", ";
		m_subExpr->Dump();
		std::cout << " }";
	}
private:
	std::string m_unaryOp;
	std::shared_ptr<Expression> m_subExpr;
	bool m_isPrefix;
};

}