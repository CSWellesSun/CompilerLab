#pragma once

#include "codegen/llvmheaders.h"
#include "common/Defs.h"
#include "parser/Ast.h"


#include <llvm/IR/Value.h>
#include "codegen/MyStructType.h"
#include <memory>
#include <vector>
#include <map>
#include <algorithm>

namespace minisolc {

struct CodeGeneratorBlock {
	llvm::Value* returnValue;
	std::map<std::string, llvm::Value*> locals;
	std::map<std::string, llvm::Type*> types;
	std::vector<std::shared_ptr<MyStructType> > structdefs;
};

class CodeGenerator {
public:
	CodeGenerator(const std::shared_ptr<BaseAST>& AstRoot) {
		m_BlockStack.push_back({nullptr, {}, {}, {}});
		createSyscall();
		generate(AstRoot);
		LOG_INFO("Codegen  Succeeds.");
	}
	void Dump() const { m_Module->print(llvm::errs(), nullptr); }
	void srctollFile(const std::string& srcfilename) const;

private:
	static std::unique_ptr<llvm::LLVMContext> m_Context;
	static std::unique_ptr<llvm::IRBuilder<>> m_Builder;
	static std::unique_ptr<llvm::Module> m_Module;
	std::vector<CodeGeneratorBlock> m_BlockStack;
	std::map<std::string, llvm::Function*> m_syscalls;

	/**
	 * @brief Generate LLVM IR from AST
	 * @param AstRoot The root of AST
	 * @param beginBlock Whether to begin a new block when encounter a block node
	 * @param isleftval Used in array and struct, determine whether returns a pointer (for leftvalue) or a value (for right value)
	 * @return The value of the AST
	 */
	llvm::Value* generate(const std::shared_ptr<BaseAST>& AstRoot, bool beginBlock = true, bool isleftval = false);

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
	std::shared_ptr<MyStructType> getStructType(const std::string& name) const {
		for(auto it = m_BlockStack.crbegin(); it != m_BlockStack.crend(); ++it) {
			auto findres = std::find_if(it->structdefs.cbegin(), it->structdefs.cend(), 
				[s = llvm::StringRef(name)](const auto& st) { return st->GetStructType()->getStructName() == s; });
			if (findres != it->structdefs.cend())
				return *findres;
		}
		return nullptr;
	}
	std::shared_ptr<MyStructType> getStructSymbolType(const std::string& name) const {
		for (auto it = m_BlockStack.rbegin(); it != m_BlockStack.rend(); ++it) {
			if (it->types.find(name) != it->types.end()) {
				return this->getStructType(it->types.at(name)->getStructName().str());
			}
		}
		return nullptr;
	}
	llvm::Value* getReturnValue() const { return m_BlockStack.back().returnValue; };
	void setSymbolValue(const std::string& name, llvm::Value* value) { m_BlockStack.back().locals[name] = value; };
	void setSymbolType(const std::string& name, llvm::Type* type) { m_BlockStack.back().types[name] = type; };
	void setReturnValue(llvm::Value* value) { m_BlockStack.back().returnValue = value; };
	void pushBlock() { m_BlockStack.push_back({nullptr, {}, {}, {}}); };
	void popBlock() { m_BlockStack.pop_back(); };


	static llvm::Constant* getInitValue(Token tok);
	static llvm::Type* getLLVMType(Token type);

	void createSyscall();
};

} // minisolc