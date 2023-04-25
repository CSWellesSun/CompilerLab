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
		auto contractPart = new ContractPartAST();
		auto contractPartPtr = std::unique_ptr<BaseAST>(contractPart);
		res = res && parseContractPart(contractPartPtr);
		if (!res) {
			m_source.setPos(pos);
			std::cout << "ContractDefinition 2: " << res << std::endl;
			return false;
		}
		contractDef->contract_parts.push_back(std::move(contractPartPtr));
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
	return res;
}

bool Parser::parseStateVariableDeclaration(std::unique_ptr<BaseAST>& in) { return false; }

bool Parser::parseFunctionDefinition(std::unique_ptr<BaseAST>& in) {
	size_t pos = m_source.pos();
	auto funcDef = std::make_unique<FunctionDefinitionAST>();
	bool res = match("function") && matchGet(Token::Identifier, funcDef->ident) && parseParameterList(funcDef->param_list);

		//    && parseStateMutability(funcDef->state_muts) && parseReturns(funcDef->returns)
		//    && parseBlock(funcDef->block);
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

	}
	res = res && match(")");
	if (res) {
		in = std::move(paramList);
	}
	return res;
}

bool Parser::parseStateMutability(std::unique_ptr<BaseAST>& in) { return false; }