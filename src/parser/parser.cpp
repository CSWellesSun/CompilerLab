#include "parser/Parser.h"
#include "parser/AST.h"
#include <memory>
#include <tuple>

using namespace minisolc;

bool Parser::parse() {
    if (m_source.error()) return false;
    return parseSourceUnit(m_root);
}

bool Parser::parseSourceUnit(std::unique_ptr<BaseAST>& in) {

    auto sourceUnit = std::make_unique<SourceUnitAST>();
    bool res = parseContractDefinition(sourceUnit->contract_def);
    in = std::move(sourceUnit);
    return res;
}

bool Parser::parseContractDefinition(std::unique_ptr<BaseAST>& in) {
    auto contractDef = std::make_unique<ContractDefinitionAST>();
    size_t pos = m_source.pos();
    bool res = match("contract") && matchGet(Token::Identifier, contractDef->ident) 
            && match("{");
    
    res = res && match("}");
    if (res) {
        in = std::move(contractDef);
        return true;
    } else {
        m_source.setPos(pos);
        return false;
    }
}