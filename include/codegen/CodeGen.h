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
        // generate(AstRoot);
    }
    // auto& GetModule () { return *m_Module; }
    std::shared_ptr<llvm::Value> generate(const std::shared_ptr<BaseAST>& AstRoot);
    llvm::Constant* GetInitValue(Token tok);

private:

    std::unique_ptr<llvm::Module> m_Module;
    std::unique_ptr<llvm::Function> m_MainFunction;
    std::vector<std::unique_ptr<llvm::Function> > m_Functions;
    std::vector<std::unique_ptr<llvm::BasicBlock> > m_Blocks;
};





// Further supplementation is needed


} // minisolc