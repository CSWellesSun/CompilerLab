#include "parser/Parser.h"
#include "parser/AST.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <tuple>


using namespace minisolc;

bool Parser::parse() {
	std::cout << "parse: " << m_source.error() << std::endl;
	if (m_source.error())
		return false;
	return parseSourceUnit(m_root);
}

bool Parser::parseSourceUnit(std::unique_ptr<BaseAST>& in) {
	auto sourceUnit = std::make_unique<SourceUnitAST>();
	bool res = parseContractDefinition(sourceUnit->contract_def);
	if (res)
		in = std::move(sourceUnit);
	std::cout << "SourceUnit: " << res << std::endl;
	return res;
}

bool Parser::parseContractDefinition(std::unique_ptr<BaseAST>& in) {
	auto contractDef = std::make_unique<ContractDefinitionAST>();
	size_t pos = m_source.pos();
	bool res = match("contract") && matchGet(Token::Identifier, contractDef->ident) && match("{");
	if (!res) {
		m_source.setPos(pos);
		std::cout << "ContractDefinition 1: " << res << std::endl;
		return false;
	}
	std::cout << m_source.curVal() << std::endl;
	while (m_source.curVal() != "" && m_source.curVal() != "}") {
		// parse contract part
		auto contractPart = std::make_unique<BaseAST>();
		res = res && parseContractPart(contractPart);
		if (!res) {
			m_source.setPos(pos);
			std::cout << "ContractDefinition 2: " << res << std::endl;
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
	std::cout << "ContractDefinition 3: " << res << std::endl;
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
	std::cout << "ContractPart: " << res << std::endl;
	return res;
}

bool Parser::parseStateVariableDeclaration(std::unique_ptr<BaseAST>& in) { return false; }

bool Parser::parseFunctionDefinition(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto funcDef = std::make_unique<FunctionDefinitionAST>();
	bool res
		= match("function") && matchGet(Token::Identifier, funcDef->ident) && parseParameterList(funcDef->param_list);

	std::cout << "before state:" << res << std::endl;
	while (m_source.curVal() == "pure" || m_source.curVal() == "view" || m_source.curVal() == "payable"
		   || m_source.curVal() == "public" || m_source.curVal() == "internal" || m_source.curVal() == "external"
		   || m_source.curVal() == "private") {
		funcDef->state_muts.push_back(m_source.curVal());
		m_source.advance();
	}
	std::cout << "before return:" << res << std::endl;

	if (m_source.curVal() == "returns") {
		match("returns");
		res = res && match("(") && parseTypeName(funcDef->return_type) && match(")");
	}
	std::cout << "before block:" << res << std::endl;
	res = res && parseBlock(funcDef->block);

	if (res) {
		in = std::move(funcDef);
	} else {
		m_source.setPos(pos);
	}
	std::cout << "FunctionDefinition: " << res << std::endl;
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
			/// NOTE:
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
	std::cout << "elementary type: " << m_source.curVal() << std::endl;
	switch (m_source.curTok()) {
		case Token::Address: [[fallthrough]];
		case Token::Bool: [[fallthrough]];
		case Token::String: [[fallthrough]];
		case Token::Var: [[fallthrough]];
		case Token::Int: [[fallthrough]];
		case Token::UInt: [[fallthrough]];
		case Token::IntM: [[fallthrough]];
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
	// while (m_source.curVal() != "" && m_source.curVal() != "}") {
	// 	auto stmt = std::make_unique<BaseAST>();
	// 	res = res && parseStatement(stmt);
	// 	if (res) {
	// 		block->stmts.push_back(std::move(stmt));
	// 	} else {
	// 		m_source.setPos(pos);
	// 		return false;
	// 	}
	// }
	res = res && match("}");
	if (res) {
		in = std::move(block);
	} else {
		m_source.setPos(pos);
	}
	return res;
}