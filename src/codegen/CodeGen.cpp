#include "codegen/CodeGen.h"
#include "common/Defs.h"
#include "parser/Ast.h"

#include <llvm-14/llvm/IR/Function.h>
#include <memory>

using namespace minisolc;

// declare static member variables
std::unique_ptr<llvm::LLVMContext> CodeGenerator::m_Context = std::make_unique<llvm::LLVMContext>();
std::unique_ptr<llvm::IRBuilder<>> CodeGenerator::m_Builder = std::make_unique<llvm::IRBuilder<>>(*m_Context);
std::unique_ptr<llvm::Module> CodeGenerator::m_Module = std::make_unique<llvm::Module>("minisolc", *m_Context);
std::map<std::string, llvm::Value*> CodeGenerator::m_NamedValues;

llvm::Constant* CodeGenerator::GetInitValue(Token tok) {
	ASSERT(isType(tok), "Invalid type!");
	switch (tok) {
	case Token::Int:
		[[fallthrough]];
	case Token::UInt:
		return m_Builder->getInt32(0);
	case Token::String:
		return nullptr;
	case Token::Bool:
		return m_Builder->getInt1(false);
	case Token::Float:
		return llvm::ConstantFP::get(m_Builder->getFloatTy(), 0.0);
	case Token::Double:
		return llvm::ConstantFP::get(m_Builder->getDoubleTy(), 0.0);
	default:
		return nullptr;
	}
}

llvm::Type* CodeGenerator::GetLLVMType(Token type) {
	ASSERT(isType(type), "Invalid type!");
	switch (type) {
	case Token::Int:
		[[fallthrough]];
	case Token::UInt:
		return llvm::Type::getInt32Ty(*m_Context);
	case Token::String:
		return nullptr;
	case Token::Bool:
		return llvm::Type::getInt1Ty(*m_Context);
	case Token::Float:
		return llvm::Type::getFloatTy(*m_Context);
	case Token::Double:
		return llvm::Type::getDoubleTy(*m_Context);
	default:
		return nullptr;
	}
}

llvm::Value* CodeGenerator::generate(const std::shared_ptr<BaseAST>& AstNode) {
	switch (AstNode->GetASTType()) {
	case ElementASTTypes::SourceUnit: {
		const SourceUnit* node = dynamic_cast<const SourceUnit*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		for (const auto& subnode: node->getSubNodes()) {
			this->generate(subnode);
		}
		return nullptr;
	}
	case ElementASTTypes::PlainVariableDefinition: {
		PlainVariableDefinition* node = dynamic_cast<PlainVariableDefinition*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		llvm::Type* type = GetLLVMType(node->GetDeclarationType()->GetType());
		llvm::Value* res = m_Builder->CreateAlloca(type, nullptr, node->GetName()); // error!
		auto& expr = node->getVarDefExpr();
		if (expr != nullptr) {
			res = m_Builder->CreateStore(generate(expr), res);
		}
		m_NamedValues[node->GetName()] = res;
		return res;
	}
	case ElementASTTypes::FunctionDefinition: {
		const FunctionDefinition* node = dynamic_cast<const FunctionDefinition*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		// Check if the function has been defined
		llvm::Function* func = m_Module->getFunction(node->GetName());
		if (func == nullptr) {
			// Create the function
			std::vector<llvm::Type*> argTypes;
			for (const auto& arg: node->GetArgs()) {
				argTypes.push_back(GetLLVMType(arg->GetDeclarationType()->GetType()));
			}
			llvm::FunctionType* funcType
				= llvm::FunctionType::get(GetLLVMType(node->GetDeclarationType()->GetType()), argTypes, false);
			func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->GetName(), m_Module.get());
			// Set names for all arguments
			unsigned idx = 0;
			for (auto& arg: func->args()) {
				arg.setName(node->GetArgs()[idx++]->GetName());
			}

			if (func == nullptr) {
				return nullptr;
			}

			if (node->GetBody() == nullptr) {
				return func;
			}

			// Create a new basic block to start insertion into.
			llvm::BasicBlock* bb = llvm::BasicBlock::Create(*m_Context, "entry", func);
			m_Builder->SetInsertPoint(bb);

			// Record the function arguments in the NamedValues map.
			m_NamedValues.clear();
			for (auto& arg: func->args()) {
				m_NamedValues[std::string(arg.getName())] = &arg;
			}

			// Generate the body of the function.
			// if (auto retVal = generate(node->GetBody())) {
			// Finish off the function.
			auto retVal = m_Builder->getInt32(0);
			m_Builder->CreateRet(retVal);

			// Validate the generated code, checking for consistency.
			llvm::verifyFunction(*func);

			return func;
			// }

			// Error reading body, remove function.
			// func->eraseFromParent();
			// return nullptr;
		}
	}
	case ElementASTTypes::FunctionCall: {
		const FunctionCall* node = dynamic_cast<const FunctionCall*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		llvm::Function* func = m_Module->getFunction(node->GetFunctionName());
		if (func == nullptr) {
			LOG_ERROR("Function %s not found.", node->GetFunctionName().c_str());
			return nullptr;
		}

		if (func->arg_size() != node->GetArgs().size()) {
			LOG_ERROR("Function %s argument size mismatch.", node->GetFunctionName().c_str());
			return nullptr;
		}

		std::vector<llvm::Value*> args;
		for (const auto& arg: node->GetArgs()) {
			args.push_back(generate(arg));
			if (args.back() == nullptr) {
				LOG_ERROR("Function %s argument generation failed.", node->GetFunctionName().c_str());
				return nullptr;
			}
		}
		LOG_INFO("here");
		return m_Builder->CreateCall(func, args, "calltmp");
	}
	case ElementASTTypes::ExpressionStatement: {
		const ExpressionStatement* node = dynamic_cast<const ExpressionStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		return generate(node->GetExpression());
	}
	default:
		// Not implemented!
		return nullptr;
	}
}
