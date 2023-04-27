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
	Declaration(std::string name, std::unique_ptr<BaseAST> type = nullptr): m_name(name), m_type(std::move(type)) {}

protected:
	std::string m_name;
	std::unique_ptr<BaseAST> m_type;
};

// SourceUnit 是 BaseAST
class SourceUnit: public BaseAST {
public:
	SourceUnit(std::unique_ptr<BaseAST> subnode): m_subnode(std::move(subnode)) {}
	void Dump() const override {
		std::cout << "SourceUnitAST { ";
		if (m_subnode)
			m_subnode->Dump();
		std::cout << " }";
	}

private:
	// 用智能指针管理对象
	std::unique_ptr<BaseAST> m_subnode;
};

class ContractDefinition: public Declaration {
public:
	// 用智能指针管理对象
	ContractDefinition(std::string name, std::vector<std::unique_ptr<BaseAST>> subnodes)
		: Declaration(name), m_subnodes(std::move(subnodes)) {}
	void Dump() const override {
		std::cout << "ContractDefinitionAST { ";
		std::cout << m_name << ", ";
		for (auto& subnode: m_subnodes) {
			subnode->Dump();
			std::cout << ", ";
		}
		std::cout << " }";
	}

private:
	std::vector<std::unique_ptr<BaseAST>> m_subnodes;
};

class VariableDeclaration: public Declaration {
public:
	VariableDeclaration(std::string name, std::unique_ptr<BaseAST> type, std::unique_ptr<BaseAST> expr)
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
	std::unique_ptr<BaseAST> m_expr; // optional
};

// FuncDef 也是 BaseAST
class FunctionDefinition: public Declaration {
public:
	FunctionDefinition(
		std::string name,
		std::unique_ptr<BaseAST> param_list,
		StateMutability state_mut,
		Visibility visibility,
		std::unique_ptr<BaseAST> return_type,
		std::unique_ptr<BaseAST> block)
		: Declaration(name, std::move(return_type)), m_param(std::move(param_list)), m_state(state_mut),
		  m_visibility(visibility), m_block(std::move(block)) {}

	void Dump() const override {
		std::cout << "FuncDefAST { ";
		std::cout << m_name << ", ";
		m_param->Dump();
		std::cout << stateMutabilityToString(m_state) << ", ";
		if (m_type)
			m_type->Dump();
		m_block->Dump();
		std::cout << " }";
	}

private:
	std::unique_ptr<BaseAST> m_param;
	StateMutability m_state;
	Visibility m_visibility;
	std::unique_ptr<BaseAST> m_block;
};

class ParameterList: public BaseAST {
public:
	ParameterList(std::vector<std::unique_ptr<BaseAST>> params): params(std::move(params)) {}
	void Dump() const override {
		std::cout << "ParameterListAST { ";
		for (auto& param: params) {
			param->Dump();
		}
		std::cout << " }";
	}

private:
	std::vector<std::unique_ptr<BaseAST>> params; // type, ident
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
	Block(std::vector<std::unique_ptr<BaseAST>> stmts): stmts(std::move(stmts)) {}
	void Dump() const override {
		std::cout << "BlockAST { ";
		for (auto& stmt: stmts) {
			stmt->Dump();
			std::cout << ", ";
		}
		std::cout << " }";
	}

private:
	std::vector<std::unique_ptr<BaseAST>> stmts;
};

class Statement: public BaseAST {
public:
	Statement(std::unique_ptr<BaseAST> child): child(std::move(child)) {}
	void Dump() const override {
		std::cout << "StatementAST { ";
		child->Dump();
		std::cout << " }";
	}

private:
	std::unique_ptr<BaseAST> child;
};

class Return: public BaseAST {
public:
	Return(std::unique_ptr<BaseAST> expr): expr(std::move(expr)) {}
	void Dump() const override {
		std::cout << "ReturnAST { ";
		if (expr)
			expr->Dump();
		std::cout << " }";
	}

private:
	std::unique_ptr<BaseAST> expr; // optional
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

}