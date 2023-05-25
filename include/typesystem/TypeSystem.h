#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "lexer/Token.h"
#include "parser/Ast.h"
#include "parser/Parser.h"
#include <map>
#include <string>

using namespace minisolc;

// analysis
// 从parser拿到AST的root，遍历这个tree，codegen

;
class TypeSystem {
public:
	TypeSystem(const Parser& parser) {
		pushMap();
		root = parser.GetAst();
		analyze();
	};


	// Function to add a new type to the type system
	void setType(std::string identifier, Type type);

	Type getType(std::string identifier);

	void pushMap() { m_maps.push_back({}); }

	void popMap() { m_maps.pop_back(); }

	void analyze();

private:
	std::vector<std::map<std::string, Type>> m_maps;
	std::shared_ptr<BaseAST> root;
};

#endif // TYPE_SYSTEM_H
