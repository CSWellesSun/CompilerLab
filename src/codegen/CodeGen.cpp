#include "codegen/CodeGen.h"
#include "parser/Ast.h"
#include "common/Defs.h"

#include <memory>

using namespace minisolc;

// declare static member variables
llvm::LLVMContext CodeGenerator::m_llvmContext {};
llvm::IRBuilder<> CodeGenerator::m_IRBuilder {CodeGenerator::m_llvmContext};

llvm::Constant* CodeGenerator::GetInitValue(Token tok) {
    ASSERT(isType(tok), "Invalid type!");
    switch (tok) {
        case Token::Int :
        [[fallthrough]]
        case Token::UInt :
            return m_IRBuilder.getInt32(0);
        case Token::String :
            return nullptr;
        case Token::Bool :
            return m_IRBuilder.getInt1(false);
        case Token::Float :
            return llvm::ConstantFP::get(m_IRBuilder.getFloatTy(), 0.0);
        case Token::Double :
            return llvm::ConstantFP::get(m_IRBuilder.getDoubleTy(), 0.0);
    }
}


std::shared_ptr<llvm::Value> CodeGenerator::generate(const std::shared_ptr<BaseAST>& AstNode) {
    switch (AstNode->GetASTType()) {
        case ElementASTTypes::SourceUnit : {
            const SourceUnit * node = dynamic_cast<const SourceUnit *>(AstNode.get());
            ASSERT(node != nullptr, "dynamic cast fails.");
            for (const auto& subnode : node->getSubNodes()) {
                this->generate(subnode);
            }
            return nullptr;
        }
        case ElementASTTypes::PlainVariableDefinition : {
            PlainVariableDefinition * node = dynamic_cast<PlainVariableDefinition *>(AstNode.get());
            ASSERT(node != nullptr, "dynamic cast fails.");
            std::shared_ptr<ElementaryTypeName> eletype = std::static_pointer_cast<ElementaryTypeName>(node->GetDeclarationType());
            std::shared_ptr<llvm::Type> type = std::reinterpret_pointer_cast<llvm::Type>(generate(eletype));
        
            
            std::shared_ptr<llvm::Value> res = std::make_shared<llvm::Value> (
                llvm::GlobalVariable(*m_Module, type.get(), false, llvm::GlobalValue::ExternalLinkage, GetInitValue(eletype->GetElementaryType()), node->GetName())
            );

            auto& expr = node->getVarDefExpr();
            if (expr != nullptr) {
                res = std::make_shared<llvm::Value>(m_IRBuilder.CreateStore(generate(expr).get(), res.get()));
            }
            return res;
        }
        // case ...
        // ...
        // QWQ
    }
}