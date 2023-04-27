#include "parser/Parser.h"
#include "parser/AST.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <tuple>


using namespace minisolc;

bool Parser::parse() {
	bool res;
	if (m_source.error())
		res = false;
	else 
		res = parseSourceUnit(m_root);
	if (res) 
		LOG_INFO("Parse Succeeds.");
	else
		LOG_WARNING("Parse Fails.");
	return res;
}

bool Parser::parseSourceUnit(std::unique_ptr<BaseAST>& in) {
	auto sourceUnit = std::make_unique<SourceUnitAST>();
	bool res = parseContractDefinition(sourceUnit->contract_def);
	if (res)
		in = std::move(sourceUnit);
	return res;
}

bool Parser::parseContractDefinition(std::unique_ptr<BaseAST>& in) {
	auto contractDef = std::make_unique<ContractDefinitionAST>();
	size_t pos = m_source.pos();
	bool res = match("contract") && matchGet(Token::Identifier, contractDef->ident) && match("{");
	if (!res) {
		m_source.setPos(pos);
		return false;
	}
	while (m_source.curVal() != "" && m_source.curVal() != "}") {
		// parse contract part
		auto contractPart = std::make_unique<BaseAST>();
		res = res && parseContractPart(contractPart);
		if (!res) {
			m_source.setPos(pos);
			return false;
		}
		contractDef->contract_parts.push_back(std::move(contractPart));
	}
	res = res && match("}");
	if (res) {
		in = std::move(contractDef);
	} else {
		m_source.setPos(pos);
	}
	return res;
}

bool Parser::parseContractPart(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto contractPart = std::make_unique<ContractPartAST>();
	bool res = parseStateVariableDeclaration(contractPart->child) || parseFunctionDefinition(contractPart->child);
	if (res) {
		in = std::move(contractPart);
	} else {
		m_source.setPos(pos);
	}
	return res;
}

bool Parser::parseStateVariableDeclaration(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto stateVarDecl = std::make_unique<StateVariableDeclarationAST>();
	bool res = parseTypeName(stateVarDecl->type) && matchGet(Token::Identifier, stateVarDecl->ident);
	if (!res) {
		m_source.setPos(pos);
		return false;
	}
	if (m_source.curVal() == "=") {
		match("=");
		res = res && parseExpression(stateVarDecl->expr);
	}
	res = res && match(";");
	if (res) {
		in = std::move(stateVarDecl);
	} else {
		m_source.setPos(pos);
	}
	return res;
}

bool Parser::parseFunctionDefinition(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto funcDef = std::make_unique<FunctionDefinitionAST>();
	bool res
		= match("function") && matchGet(Token::Identifier, funcDef->ident) && parseParameterList(funcDef->param_list);

	while (m_source.curVal() == "pure" || m_source.curVal() == "view" || m_source.curVal() == "payable"
		   || m_source.curVal() == "public" || m_source.curVal() == "internal" || m_source.curVal() == "external"
		   || m_source.curVal() == "private") {
		funcDef->state_muts.push_back(m_source.curVal());
		m_source.advance();
	}

	if (m_source.curVal() == "returns") {
		match("returns");
		res = res && match("(") && parseTypeName(funcDef->return_type) && match(")");
	}
	res = res && parseBlock(funcDef->block);

	if (res) {
		in = std::move(funcDef);
	} else {
		m_source.setPos(pos);
	}
	return res;
}

bool Parser::parseParameterList(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto paramList = std::make_unique<ParameterListAST>();
	bool res = match("(");
	if (m_source.curVal() != "" && m_source.curVal() != ")") {
		// 1st parameter
		auto param = std::make_unique<BaseAST>();
		std::string ident;
		res = res && parseTypeName(param) && matchGet(Token::Identifier, ident);
		if (res) {
			paramList->params.push_back(std::make_pair(std::move(param), ident));
			// other parameters
			while (m_source.curVal() != "" && m_source.curVal() != ")") {
				res = res && match(",") && parseTypeName(param) && matchGet(Token::Identifier, ident);
				if (res) {
					paramList->params.push_back(std::make_pair(std::move(param), ident));
				} else {
					m_source.setPos(pos);
					return false;
				}
			}
		} else {
			m_source.setPos(pos);
			return false;
		}
	}
	res = res && match(")");
	if (res) {
		in = std::move(paramList);
	} else {
		m_source.setPos(pos);
	}
	return res;
}

bool Parser::parseTypeName(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto typeName = std::make_unique<TypeNameAST>();
	bool res = parseElementaryTypeName(typeName->elem_type);
	if (res) {
		in = std::move(typeName);
	} else {
		m_source.setPos(pos);
	}
	return res;
}

bool Parser::parseElementaryTypeName(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto elemTypeName = std::make_unique<ElementaryTypeNameAST>();
	switch (m_source.curTok()) {
	case Token::Address:
		[[fallthrough]];
	case Token::Bool:
		[[fallthrough]];
	case Token::String:
		[[fallthrough]];
	case Token::Var:
		[[fallthrough]];
	case Token::Int:
		[[fallthrough]];
	case Token::UInt:
		[[fallthrough]];
	case Token::IntM:
		[[fallthrough]];
	case Token::UIntM:
		elemTypeName->type = m_source.curVal();
		in = std::move(elemTypeName);
		m_source.advance();
		return true;
	default:
		m_source.setPos(pos);
		return false;
	}
}

bool Parser::parseBlock(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto block = std::make_unique<BlockAST>();
	bool res = match("{");
	while (m_source.curVal() != "" && m_source.curVal() != "}") {
		auto stmt = std::make_unique<BaseAST>();
		res = res && parseStatement(stmt);
		if (res) {
			block->stmts.push_back(std::move(stmt));
		} else {
			m_source.setPos(pos);
			return false;
		}
	}
	res = res && match("}");
	if (res) {
		in = std::move(block);
	} else {
		m_source.setPos(pos);
	}
	return res;
}

bool Parser::parseStatement(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto stmt = std::make_unique<StatementAST>();
	bool res = parseReturn(stmt->child) && match(";");
	if (res) {
		in = std::move(stmt);
		return true;
	} else {
		m_source.setPos(pos);
		return false;
	}
}

bool Parser::parseReturn(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto ret = std::make_unique<ReturnAST>();
	bool res = match("return");
	if (m_source.curVal() != ";") {
		res = res && parseExpression(ret->expr);
	}
	if (res) {
		in = std::move(ret);
	} else {
		m_source.setPos(pos);
	}
	return res;
}

bool Parser::parseExpression(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto expr = std::make_unique<ExpressionAST>();
	bool res = parsePrimaryExpression(expr->child);
	if (res) {
		in = std::move(expr);
	} else {
		m_source.setPos(pos);
	}
	return res;
}

bool Parser::parsePrimaryExpression(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto primExpr = std::make_unique<PrimaryExpressionAST>();
	switch (m_source.curTok()) {
	case Token::Identifier:
		[[fallthrough]];
	case Token::TrueLiteral:
		[[fallthrough]];
	case Token::FalseLiteral:
		[[fallthrough]];
	case Token::StringLiteral:
		primExpr->child = m_source.curVal();
		m_source.advance();
		in = std::move(primExpr);
		return true;
	case Token::Number:
		primExpr->child = m_source.curVal();
		m_source.advance();
		// Number Unit
		switch (m_source.curTok()) {
		case Token::SubWei:
			[[fallthrough]];
		case Token::SubGwei:
			[[fallthrough]];
		case Token::SubEther:
			[[fallthrough]];
		case Token::SubSecond:
			[[fallthrough]];
		case Token::SubMinute:
			[[fallthrough]];
		case Token::SubHour:
			[[fallthrough]];
		case Token::SubDay:
			[[fallthrough]];
		case Token::SubWeek:
			[[fallthrough]];
		case Token::SubYear:
			primExpr->child += m_source.curVal();
			m_source.advance();
			break;
		default:
			break;
		}
		in = std::move(primExpr);
		return true;
	default:
		m_source.setPos(pos);
		return false;
	}
}