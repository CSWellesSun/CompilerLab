#pragma once

#include "codegen/llvmheaders.h"
#include "common/Defs.h"
#include "parser/Ast.h"


#include <llvm/IR/Value.h>
#include <memory>
#include <vector>

namespace minisolc {

struct CodeGeneratorBlock {
	llvm::Value* returnValue;
	std::map<std::string, llvm::Value*> locals;
	std::map<std::string, llvm::Type*> types;
};

class CodeGenerator {
public:
	CodeGenerator(const std::shared_ptr<BaseAST>& AstRoot) {
		m_BlockStack.push_back({nullptr, {}, {}});

		generate(AstRoot);
		LOG_INFO("Codegen  Succeeds.");
	}
	void Dump() const { m_Module->print(llvm::errs(), nullptr); }

private:
	static std::unique_ptr<llvm::LLVMContext> m_Context;
	static std::unique_ptr<llvm::IRBuilder<>> m_Builder;
	static std::unique_ptr<llvm::Module> m_Module;
	std::vector<CodeGeneratorBlock> m_BlockStack;

	/**
	 * @brief Generate LLVM IR from AST
	 * @param AstRoot The root of AST
	 * @param beginBlock Whether to begin a new block when encounter a block node
	 * @return The value of the AST
	 */
	llvm::Value* generate(const std::shared_ptr<BaseAST>& AstRoot, bool beginBlock = true);

	llvm::Value* getSymbolValue(const std::string& name) const {
		for (auto it = m_BlockStack.rbegin(); it != m_BlockStack.rend(); ++it) {
			if (it->locals.find(name) != it->locals.end()) {
				return it->locals.at(name);
			}
		}
		return nullptr;
	};
	llvm::Type* getSymbolType(const std::string& name) const {
		for (auto it = m_BlockStack.rbegin(); it != m_BlockStack.rend(); ++it) {
			if (it->types.find(name) != it->types.end()) {
				return it->types.at(name);
			}
		}
		return nullptr;
	};
	llvm::Value* getReturnValue() const { return m_BlockStack.back().returnValue; };
	void setSymbolValue(const std::string& name, llvm::Value* value) { m_BlockStack.back().locals[name] = value; };
	void setSymbolType(const std::string& name, llvm::Type* type) { m_BlockStack.back().types[name] = type; };
	void setReturnValue(llvm::Value* value) { m_BlockStack.back().returnValue = value; };
	void pushBlock() { m_BlockStack.push_back({nullptr, {}, {}}); };
	void popBlock() { m_BlockStack.pop_back(); };


	llvm::Constant* getInitValue(Token tok);
	llvm::Type* getLLVMType(Token type);
};

} // minisolc