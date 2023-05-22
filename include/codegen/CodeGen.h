#pragma once

#include "codegen/llvmheaders.h"
#include "parser/Ast.h"
#include "common/Defs.h"

#include <memory>
#include <vector>

namespace minisolc {

// Only a rough draft
class CodeGenerator {
public:
	CodeGenerator(const std::shared_ptr<BaseAST>& AstRoot) {
		generate(AstRoot);
        LOG_INFO("Codegen  Succeeds.");
	}
    void Dump() const {
        m_Module->print(llvm::errs(), nullptr);
    }

private:
    static std::unique_ptr<llvm::LLVMContext> m_Context;
    static std::unique_ptr<llvm::IRBuilder<>> m_Builder;
    static std::unique_ptr<llvm::Module> m_Module;
    static std::map<std::string, llvm::Value*> m_NamedValues;

	llvm::Value* generate(const std::shared_ptr<BaseAST>& AstRoot);

	llvm::Constant* GetInitValue(Token tok);
    llvm::Type* GetLLVMType(Token type);
};


// Further supplementation is needed


} // minisolc