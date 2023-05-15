#pragma once

#include "codegen/llvmheaders.h"

#include "parser/Ast.h"

#include <memory>
#include <vector>

namespace minisolc {

// Only a rough draft
class CodeGenerator {
public:
    static llvm::LLVMContext m_llvmContext;
    static llvm::IRBuilder<> m_IRBuilder;

    CodeGenerator(const std::shared_ptr<BaseAST>& AstRoot) {

    }
    auto& GetModule () { return *m_Module; }

private:
    std::unique_ptr<llvm::Module> m_Module;
    std::unique_ptr<llvm::Function> m_MainFunction;
    std::vector<std::unique_ptr<llvm::Function> > m_Functions;
    std::vector<std::unique_ptr<llvm::BasicBlock> > m_Blocks;
};

// llvm::LLVMContext CodeGenerator::m_llvmContext {};
// llvm::IRBuilder<> CodeGenerator::m_IRBuilder {CodeGenerator::m_llvmContext};

// Further supplementation is needed


} // minisolc