#pragma once

#include "common/Defs.h"
#include "lexer/Token.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>


namespace minisolc {

/* Each final AST records its own type
   for subsequent code generation. */
enum class ElementASTTypes {
	Invalid = 0,
	SourceUnit,
	PlainVariableDefinition,
	ArrayDefinition,
	StructDefinition,
	ParameterList, // Maybe useless, remove later
	Block,
	FunctionDefinition,
	ElementaryTypeName, // Maybe useless, remove later
	ReturnStatement,
	Identifier,
	BooleanLiteral,
	StringLiteral,
	NumberLiteral,
	Assignment,
	BinaryOp,
	UnaryOp,
	IfStatement,
	WhileStatement,
	ForStatement,
	DoWhileStatement,
	BreakStatement,
	ContinueStatement,
	ExpressionStatement,
	IndexAccess,
	FunctionCall,
	MemberAccess,
};

// Base class for all ASTs
class BaseAST {
public:
	virtual ~BaseAST() = default;

	virtual void Dump(size_t, size_t) const {};
	ElementASTTypes GetASTType() const { return m_ASTType; }

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

	static std::string astColor(size_t depth) {
		static const std::vector<std::string> colors = {GREEN, YELLOW, BLUE, MAGENTA, CYAN};
		return colors[depth % colors.size()];
	}

	static size_t set(size_t mask, size_t pos) { return mask | (1 << pos); }

	static size_t unset(size_t mask, size_t pos) { return mask & ~(1 << pos); }

	ElementASTTypes m_ASTType = ElementASTTypes::Invalid;
};

class Statement: public BaseAST {};
class SimpleStatement: public Statement {};
class Expression: public BaseAST {};
class TypeName: public BaseAST {
public:
	virtual Token GetType() = 0;
};

class Declaration {
public:
	Declaration(std::string name, std::shared_ptr<TypeName> type = nullptr): m_name(name), m_type(std::move(type)) {}
	const std::string& GetName() const { return m_name; }
	const std::shared_ptr<TypeName>& GetDeclarationType() const { return m_type; }

protected:
	std::string m_name;
	std::shared_ptr<TypeName> m_type;
};

class SourceUnit final: public BaseAST {
public:
	SourceUnit(std::vector<std::shared_ptr<BaseAST>> subnodes): m_subnodes(std::move(subnodes)) {
		m_ASTType = ElementASTTypes::SourceUnit;
	}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "SourceUnitAST" << RESET << '\n';

		if (!m_subnodes.empty()) {
			printIndent(depth + 1, mask);
			std::cout << "childs:" << '\n';
			mask = set(mask, depth + 2);
			for (const auto& subnode: m_subnodes) {
				if (subnode == m_subnodes.back())
					mask = unset(mask, depth + 2);
				subnode->Dump(depth + 2, mask);
			}
		}
	}

	const auto& getSubNodes() const { return m_subnodes; } // for tranverse

private:
	std::vector<std::shared_ptr<BaseAST>> m_subnodes;
};

class VariableDefinition: public Declaration, public SimpleStatement {
public:
	VariableDefinition(std::string name, std::shared_ptr<TypeName> type): Declaration(name, std::move(type)) {}
};

class PlainVariableDefinition final: public VariableDefinition {
public:
	PlainVariableDefinition(std::string name, std::shared_ptr<TypeName> type, std::shared_ptr<Expression> expr)
		: VariableDefinition(name, std::move(type)), m_expr(std::move(expr)) {
		m_ASTType = ElementASTTypes::PlainVariableDefinition;
	}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "PlainVariableDefinitionAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "name: " << m_name << '\n';

		if (!m_expr)
			mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "type: " << '\n';

		m_type->Dump(depth + 2, mask);
		if (m_expr) {
			mask = unset(mask, depth + 1);
			printIndent(depth + 1, mask);
			std::cout << "expr: " << '\n';
			m_expr->Dump(depth + 2, mask);
		}
	}
	const auto& getVarDefExpr() const { return m_expr; }

private:
	std::shared_ptr<Expression> m_expr; // optional
};

class ParameterList final: public BaseAST {
public:
	ParameterList(std::vector<std::shared_ptr<VariableDefinition>> params): params(std::move(params)) {
		m_ASTType = ElementASTTypes::ParameterList;
	}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ParameterListAST" << RESET << '\n';

		if (!params.empty()) {
			printIndent(depth + 1, mask);
			std::cout << "params:" << '\n';
			mask = set(mask, depth + 2);
			for (const auto& param: params) {
				if (param == params.back())
					mask = unset(mask, depth + 2);
				param->Dump(depth + 2, mask);
			}
		}
	}
	const auto& GetArgs() const { return params; }

private:
	std::vector<std::shared_ptr<VariableDefinition>> params; // type, ident
};

class Block final: public Statement {
public:
	Block(std::vector<std::shared_ptr<Statement>> stmts): m_stmts(std::move(stmts)) {
		m_ASTType = ElementASTTypes::Block;
	}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "BlockAST" << RESET << '\n';

		if (!m_stmts.empty()) {
			printIndent(depth + 1, mask);
			std::cout << "childs:" << '\n';
			mask = set(mask, depth + 2);
			for (const auto& stmt: m_stmts) {
				if (stmt == m_stmts.back())
					mask = unset(mask, depth + 2);
				stmt->Dump(depth + 2, mask);
			}
		}
	}

	const auto& GetStatements() const { return m_stmts; }

private:
	std::vector<std::shared_ptr<Statement>> m_stmts;
};

class FunctionDefinition final: public Declaration, public BaseAST {
public:
	FunctionDefinition(
		std::string name,
		std::shared_ptr<ParameterList> param_list,
		Visibility visibility,
		std::shared_ptr<TypeName> return_type,
		std::shared_ptr<Block> block)
		: Declaration(name, std::move(return_type)), m_param(std::move(param_list)), m_visibility(visibility),
		  m_block(std::move(block)) {
		m_ASTType = ElementASTTypes::FunctionDefinition;
	}

	const std::vector<std::shared_ptr<VariableDefinition>> GetArgs() const {
		if (!m_param)
			return {};
		return m_param->GetArgs();
	}

	const std::shared_ptr<Block> GetBody() const { return m_block; }

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "FuncDefAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "name: " << m_name << '\n';
		if (m_param) {
			printIndent(depth + 1, mask);
			std::cout << "params: " << '\n';
			m_param->Dump(depth + 2, mask);
		}

		if (m_type) {
			printIndent(depth + 1, mask);
			std::cout << "return type:" << '\n';
			m_type->Dump(depth + 2, mask);
		}

		if (!m_block)
			mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "visibility: " << visibilityToString(m_visibility) << '\n';

		if (m_block) {
			mask = unset(mask, depth + 1);
			printIndent(depth + 1, mask);
			std::cout << "block:" << '\n';

			m_block->Dump(depth + 2, mask);
		}
	}

private:
	std::shared_ptr<ParameterList> m_param;
	Visibility m_visibility;
	std::shared_ptr<Block> m_block;
};


class ElementaryTypeName final: public TypeName {
public:
	ElementaryTypeName(Token type): m_type(type) { m_ASTType = ElementASTTypes::ElementaryTypeName; }
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ElementaryTypeNameAST" << RESET << '\n';

		printIndent(depth + 1, mask);
		std::cout << "type: " << tokenToString(m_type) << '\n';
	}
	Token GetType() override { return m_type; }

private:
	Token m_type;
};


class ReturnStatement final: public Statement {
public:
	ReturnStatement(std::shared_ptr<Expression> expr): m_expr(std::move(expr)) {
		m_ASTType = ElementASTTypes::ReturnStatement;
	}

	std::shared_ptr<Expression> GetExpr() const { return m_expr; }

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ReturnAST" << RESET << '\n';
		if (m_expr) {
			printIndent(depth + 1, mask);
			std::cout << "expr: " << '\n';

			m_expr->Dump(depth + 2, mask);
		}
	}

private:
	std::shared_ptr<Expression> m_expr; // optional
};

class PrimaryExpression: public Expression {
public:
	PrimaryExpression(std::string value): m_value(value) {}

	const std::string& GetValue() const { return m_value; }

protected:
	std::string m_value;
};

class Identifier final: public PrimaryExpression {
public:
	Identifier(std::string value): PrimaryExpression(value) { m_ASTType = ElementASTTypes::Identifier; }
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "IdentifierAST" << RESET << '\n';

		printIndent(depth + 1, mask);
		std::cout << "value: " << m_value << '\n';
	}
};

class BooleanLiteral final: public PrimaryExpression {
public:
	BooleanLiteral(std::string value): PrimaryExpression(value) { m_ASTType = ElementASTTypes::BooleanLiteral; }
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "BooleanLiteralAST" << RESET << '\n';

		printIndent(depth + 1, mask);
		std::cout << "value: " << m_value << '\n';
	}
};

class StringLiteral final: public PrimaryExpression {
public:
	StringLiteral(std::string value): PrimaryExpression(value) { m_ASTType = ElementASTTypes::StringLiteral; }
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "StringLiteralAST" << RESET << '\n';

		printIndent(depth + 1, mask);
		std::cout << "value: " << m_value << '\n';
	}
};

class NumberLiteral final: public PrimaryExpression {
public:
	NumberLiteral(std::string value /*, std::string unit = ""*/): PrimaryExpression(value) /*, m_unit(unit)*/ {
		m_ASTType = ElementASTTypes::NumberLiteral;
	}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "NumberLiteralAST" << RESET << '\n';

		printIndent(depth + 1, mask);
		std::cout << "value: " << m_value << '\n';
	}

	// private:
	// 	std::string m_unit;
};

class ArrayDefinition final: public VariableDefinition {
public:
	ArrayDefinition(std::string name, std::shared_ptr<TypeName> type, std::shared_ptr<Expression> size)
		: VariableDefinition(name, std::move(type)), m_size(std::move(size)) {
		m_ASTType = ElementASTTypes::ArrayDefinition;
	}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ArrayDefinitionAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "name: " << m_name << '\n';

		printIndent(depth + 1, mask);
		std::cout << "type: " << '\n';

		m_type->Dump(depth + 2, mask);
		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "size: " << '\n';
		m_size->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<Expression> m_size;
};

class StructDefinition final: public VariableDefinition {
public:
	StructDefinition(std::string name, std::vector<std::shared_ptr<VariableDefinition>> memList)
		: VariableDefinition(name, std::make_shared<ElementaryTypeName>(Token::Struct)), m_MemList(std::move(memList)) {
		m_ASTType = ElementASTTypes::StructDefinition;
	}
	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "StructDefinitionAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		if (m_MemList.empty()) {
			mask = unset(mask, depth + 1);
		}
		printIndent(depth + 1, mask);
		std::cout << "name: " << m_name << '\n';

		mask = unset(mask, depth + 1);
		if (!m_MemList.empty()) {
			printIndent(depth + 1, mask);
			std::cout << "members:" << '\n';
			mask = set(mask, depth + 2);
			for (auto iter = m_MemList.cbegin(); iter != m_MemList.cend(); ++iter) {
				if (iter + 1 == m_MemList.cend())
					mask = unset(mask, depth + 2);
				(*iter)->Dump(depth + 2, mask);
			}
		}
	}

private:
	std::vector<std::shared_ptr<VariableDefinition>> m_MemList;
};

class Assignment final: public Expression {
public:
	Assignment(std::shared_ptr<Expression> lhs, Token assignOp, std::shared_ptr<Expression> rhs)
		: m_leftHandSide(std::move(lhs)), m_assigmentOp(std::move(assignOp)), m_rightHandSide(std::move(rhs)) {
		m_ASTType = ElementASTTypes::Assignment;
	}

	std::shared_ptr<Expression> GetLeftHand() const { return m_leftHandSide; }
	Token GetAssigmentOp() const { return m_assigmentOp; }
	std::shared_ptr<Expression> GetRightHand() const { return m_rightHandSide; }

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "AssignmentAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "leftHandSide: " << '\n';

		m_leftHandSide->Dump(depth + 2, mask);

		printIndent(depth + 1, mask);
		std::cout << "assigmentOp: " << tokenToString(m_assigmentOp) << '\n';

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "rightHandSide: " << '\n';

		m_rightHandSide->Dump(depth + 2, mask);
	}


private:
	std::shared_ptr<Expression> m_leftHandSide;
	Token m_assigmentOp;
	std::shared_ptr<Expression> m_rightHandSide;
};

class BinaryOp final: public Expression {
public:
	BinaryOp(std::shared_ptr<Expression> lhs, Token binaryOp, std::shared_ptr<Expression> rhs)
		: m_leftHandSide(std::move(lhs)), m_binaryOp(std::move(binaryOp)), m_rightHandSide(std::move(rhs)) {
		m_ASTType = ElementASTTypes::BinaryOp;
	}

	std::shared_ptr<Expression> GetLeftHand() const { return m_leftHandSide; }
	std::shared_ptr<Expression> GetRightHand() const { return m_rightHandSide; }
	Token GetOp() const { return m_binaryOp; }

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "BinaryOpAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "leftHandSide: " << '\n';

		m_leftHandSide->Dump(depth + 2, mask);

		printIndent(depth + 1, mask);
		std::cout << "binaryOp: " << tokenToString(m_binaryOp) << '\n';

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "rightHandSide: " << '\n';

		m_rightHandSide->Dump(depth + 2, mask);
	}


private:
	std::shared_ptr<Expression> m_leftHandSide;
	Token m_binaryOp;
	std::shared_ptr<Expression> m_rightHandSide;
};

class UnaryOp final: public Expression {
public:
	UnaryOp(Token unaryOp, std::shared_ptr<Expression> subExpr, bool isPrefix)
		: m_unaryOp(std::move(unaryOp)), m_subExpr(std::move(subExpr)), m_isPrefix(isPrefix) {
		m_ASTType = ElementASTTypes::UnaryOp;
	}

	Token GetOp() const { return m_unaryOp; }
	std::shared_ptr<Expression> GetExpr() const { return m_subExpr; }
	bool IsPrefix() const { return m_isPrefix; }

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "UnaryOpAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "unaryOp: " << (m_isPrefix ? "prefix " : "postfix ") << tokenToString(m_unaryOp) << '\n';

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "subExpr: " << '\n';

		m_subExpr->Dump(depth + 2, mask);
	}

private:
	Token m_unaryOp;
	std::shared_ptr<Expression> m_subExpr;
	bool m_isPrefix;
};

class IfStatement final: public Statement {
public:
	IfStatement(
		std::shared_ptr<Expression> condition,
		std::shared_ptr<Statement> thenStatement,
		std::shared_ptr<Statement> elseStatement = nullptr)
		: m_condition(std::move(condition)), m_thenStatement(std::move(thenStatement)),
		  m_elseStatement(std::move(elseStatement)) {
		m_ASTType = ElementASTTypes::IfStatement;
	}

	std::shared_ptr<Expression> GetCondition() const { return m_condition; }
	std::shared_ptr<Statement> GetThenStatement() const { return m_thenStatement; }
	std::shared_ptr<Statement> GetElseStatement() const { return m_elseStatement; }

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "IfStatementAST" << RESET << '\n';

		if (m_thenStatement) {
			mask = set(mask, depth + 1);
		}
		printIndent(depth + 1, mask);
		std::cout << "condition: " << '\n';

		m_condition->Dump(depth + 2, mask);

		if (m_thenStatement) {
			if (!m_elseStatement) {
				mask = unset(mask, depth + 1);
			}
			printIndent(depth + 1, mask);
			std::cout << "thenStatement: " << '\n';

			m_thenStatement->Dump(depth + 2, mask);
		}

		if (m_elseStatement) {
			mask = unset(mask, depth + 1);
			printIndent(depth + 1, mask);
			std::cout << "elseStatement: " << '\n';

			m_elseStatement->Dump(depth + 2, mask);
		}
	}

private:
	std::shared_ptr<Expression> m_condition;
	std::shared_ptr<Statement> m_thenStatement;
	std::shared_ptr<Statement> m_elseStatement;
};

class WhileStatement final: public Statement {
public:
	WhileStatement(std::shared_ptr<Expression> condition, std::shared_ptr<Statement> body)
		: m_condition(std::move(condition)), m_body(std::move(body)) {
		m_ASTType = ElementASTTypes::WhileStatement;
	}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "WhileStatementAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "condition: " << '\n';

		m_condition->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "body: " << '\n';

		m_body->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<Expression> m_condition;
	std::shared_ptr<Statement> m_body;
};

class ForStatement final: public Statement {
public:
	ForStatement(
		std::shared_ptr<SimpleStatement> init,
		std::shared_ptr<Expression> condition,
		std::shared_ptr<Expression> update,
		std::shared_ptr<Statement> body)
		: m_init(std::move(init)), m_condition(std::move(condition)), m_update(std::move(update)),
		  m_body(std::move(body)) {
		m_ASTType = ElementASTTypes::ForStatement;
	}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ForStatementAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "init: " << '\n';

		m_init->Dump(depth + 2, mask);

		printIndent(depth + 1, mask);
		std::cout << "condition: " << '\n';

		m_condition->Dump(depth + 2, mask);

		printIndent(depth + 1, mask);
		std::cout << "update: " << '\n';

		m_update->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "body: " << '\n';

		m_body->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<SimpleStatement> m_init;
	std::shared_ptr<Expression> m_condition;
	std::shared_ptr<Expression> m_update;
	std::shared_ptr<Statement> m_body;
};

class DoWhileStatement final: public Statement {
public:
	DoWhileStatement(std::shared_ptr<Expression> condition, std::shared_ptr<Statement> body)
		: m_body(std::move(body)), m_condition(std::move(condition)) {
		m_ASTType = ElementASTTypes::DoWhileStatement;
	}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "DoWhileStatementAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "body: " << '\n';

		m_body->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "condition: " << '\n';

		m_condition->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<Statement> m_body;
	std::shared_ptr<Expression> m_condition;
};

class BreakStatement final: public Statement {
public:
	BreakStatement() { m_ASTType = ElementASTTypes::BreakStatement; };

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "BreakStatementAST" << RESET << '\n';
	}
};

class ContinueStatement final: public Statement {
public:
	ContinueStatement() { m_ASTType = ElementASTTypes::ContinueStatement; };

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ContinueStatementAST" << RESET << '\n';
	}
};

class ExpressionStatement final: public SimpleStatement {
public:
	ExpressionStatement(std::shared_ptr<Expression> expr): m_expr(std::move(expr)) {
		m_ASTType = ElementASTTypes::ExpressionStatement;
	}

	const std::shared_ptr<Expression>& GetExpr() const { return m_expr; }

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "ExpressionStatementAST" << RESET << '\n';

		printIndent(depth + 1, mask);
		std::cout << "expr: " << '\n';

		m_expr->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<Expression> m_expr;
};

class IndexAccess final: public Expression {
public:
	IndexAccess(std::shared_ptr<Expression> expr, std::shared_ptr<Expression> index)
		: m_expr(std::move(expr)), m_index(std::move(index)) {
		m_ASTType = ElementASTTypes::IndexAccess;
	}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "IndexAccessAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "expr: " << '\n';

		m_expr->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "index: " << '\n';

		m_index->Dump(depth + 2, mask);
	}

private:
	std::shared_ptr<Expression> m_expr;
	std::shared_ptr<Expression> m_index;
};

class FunctionCall final: public Expression {
public:
	FunctionCall(std::shared_ptr<Expression> expr, std::vector<std::shared_ptr<Expression>> args)
		: m_expr(std::move(expr)), m_args(std::move(args)) {
		m_ASTType = ElementASTTypes::FunctionCall;
	}

	std::string GetFunctionName() const { return reinterpret_cast<Identifier*>(m_expr.get())->GetValue(); }

	std::vector<std::shared_ptr<Expression>> GetArgs() const { return m_args; }

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "FunctionCallAST" << RESET << '\n';

		if (!m_args.empty())
			mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "expr: " << '\n';

		m_expr->Dump(depth + 2, mask);

		if (!m_args.empty()) {
			mask = unset(mask, depth + 1);
			printIndent(depth + 1, mask);
			std::cout << "args: " << '\n';
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

class MemberAccess final: public Expression {
public:
	MemberAccess(std::shared_ptr<Expression> expr, std::string member)
		: m_expr(std::move(expr)), m_member(std::move(member)) {
		m_ASTType = ElementASTTypes::MemberAccess;
	}

	void Dump(size_t depth, size_t mask) const override {
		printIndent(depth, mask);
		std::cout << astColor(depth) << "MemberAccessAST" << RESET << '\n';

		mask = set(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "expr: " << '\n';

		m_expr->Dump(depth + 2, mask);

		mask = unset(mask, depth + 1);
		printIndent(depth + 1, mask);
		std::cout << "member: " << m_member << '\n';
	}

private:
	std::shared_ptr<Expression> m_expr;
	std::string m_member;
};


}