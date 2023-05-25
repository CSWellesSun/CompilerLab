#include "codegen/CodeGen.h"
#include "common/Defs.h"
#include "parser/Ast.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <memory>

using namespace minisolc;

// declare static member variables
std::unique_ptr<llvm::LLVMContext> CodeGenerator::m_Context = std::make_unique<llvm::LLVMContext>();
std::unique_ptr<llvm::IRBuilder<>> CodeGenerator::m_Builder = std::make_unique<llvm::IRBuilder<>>(*m_Context);
std::unique_ptr<llvm::Module> CodeGenerator::m_Module = std::make_unique<llvm::Module>("minisolc", *m_Context);

llvm::Constant* CodeGenerator::getInitValue(Token tok) {
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

llvm::Type* CodeGenerator::getLLVMType(Token type) {
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
	case Token::Void:
		return llvm::Type::getVoidTy(*m_Context);
	default:
		return nullptr;
	}
}

llvm::Value* CodeGenerator::generate(const std::shared_ptr<BaseAST>& AstNode, bool beginBlock) {
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
		llvm::Type* type = getLLVMType(node->GetDeclarationType()->GetType());
		llvm::Value* res = m_Builder->CreateAlloca(type, nullptr, node->GetName());
		setSymbolValue(node->GetName(), res);
		setSymbolType(node->GetName(), type);

		auto& expr = node->getVarDefExpr();
		if (expr != nullptr) {
			res = generate(
				std::make_shared<Assignment>(std::make_shared<Identifier>(node->GetName()), Token::Assign, expr));
		} else {
			// initialize the variable, maybe change later
			llvm::Value* value = getInitValue(node->GetDeclarationType()->GetType());
			m_Builder->CreateStore(value, res);
		}

		return res;
	}
	case ElementASTTypes::ArrayDefinition: {
		return nullptr;
	}
	case ElementASTTypes::StructDefinition: {
		return nullptr;
	}
	case ElementASTTypes::Block: {
		if (beginBlock) {
			pushBlock();
		}

		const Block* node = dynamic_cast<const Block*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		for (const auto& subnode: node->GetStatements()) {
			this->generate(subnode);
		}

		if (beginBlock) {
			popBlock();
		}
		return nullptr;
	}
	case ElementASTTypes::FunctionDefinition: {
		const FunctionDefinition* node = dynamic_cast<const FunctionDefinition*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");

		// Check if the function has been defined
		llvm::Function* func = m_Module->getFunction(node->GetName());
		if (func != nullptr)
			return nullptr;

		// Create the function
		std::vector<llvm::Type*> argTypes;
		for (const auto& arg: node->GetArgs()) {
			argTypes.push_back(getLLVMType(arg->GetDeclarationType()->GetType()));
		}
		llvm::FunctionType* funcType
			= llvm::FunctionType::get(getLLVMType(node->GetDeclarationType()->GetType()), argTypes, false);
		func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->GetName(), m_Module.get());
		// Set names for all arguments
		unsigned idx = 0;
		for (auto& arg: func->args()) {
			arg.setName(node->GetArgs()[idx++]->GetName());
		}

		if (func == nullptr)
			return nullptr;

		if (node->GetBody() == nullptr)
			return func;

		// Create a new basic block to start insertion into.
		llvm::BasicBlock* bb = llvm::BasicBlock::Create(*m_Context, "entry", func);
		m_Builder->SetInsertPoint(bb);
		pushBlock();

		// Record the function arguments in the Symbol map.
		for (auto& arg: func->args()) {
			setSymbolValue(std::string(arg.getName()), &arg);
			setSymbolType(std::string(arg.getName()), arg.getType());
		}

		// Generate the body of the function.
		generate(node->GetBody(), false);
		if (func->getReturnType()->isVoidTy()) {
			if (m_Builder->GetInsertBlock()->getTerminator() == nullptr)
				m_Builder->CreateRetVoid();
		}
		m_Builder->ClearInsertionPoint();
		popBlock();

		// Validate the generated code, checking for consistency.
		llvm::verifyFunction(*func);

		return func;
		// Error reading body, remove function.
		// func->eraseFromParent();
		// return nullptr;
	}
	case ElementASTTypes::ReturnStatement: {
		const ReturnStatement* node = dynamic_cast<const ReturnStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		auto expr = node->GetExpr();
		if (expr == nullptr) {
			return m_Builder->CreateRetVoid();
		}
		llvm::Value* retVal = generate(expr);
		setReturnValue(retVal);
		return m_Builder->CreateRet(retVal);
	}
	case ElementASTTypes::Identifier: {
		std::string name = dynamic_cast<const Identifier*>(AstNode.get())->GetValue();
		llvm::Value* value = getSymbolValue(name);
		llvm::Type* type = getSymbolType(name);
		if (value == nullptr) {
			return nullptr;
		}
		return m_Builder->CreateLoad(type, value);
	}
	case ElementASTTypes::BooleanLiteral: {
		std::string valueString = dynamic_cast<const BooleanLiteral*>(AstNode.get())->GetValue();
		bool value = valueString == "true" ? true : false;
		return llvm::ConstantInt::get(m_Builder->getInt1Ty(), value);
	}
	case ElementASTTypes::StringLiteral: {
		return nullptr;
	}
	case ElementASTTypes::NumberLiteral: {
		/// TODO: check its type
		std::string valueString = dynamic_cast<const NumberLiteral*>(AstNode.get())->GetValue();
		double value = std::stod(valueString);
		return llvm::ConstantFP::get(m_Builder->getDoubleTy(), value);
	}
	case ElementASTTypes::Assignment: {
		const Assignment* node = dynamic_cast<const Assignment*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		const Identifier* leftHand = dynamic_cast<const Identifier*>(node->GetLeftHand().get());
		ASSERT(leftHand != nullptr, "dynamic cast fails.");
		llvm::Value* leftHandValue = getSymbolValue(leftHand->GetValue());
		llvm::Value* rightHandValue = generate(node->GetRightHand());
		Token assignmentOp = node->GetAssigmentOp();
		if (assignmentOp == Token::Assign)
			return m_Builder->CreateStore(rightHandValue, leftHandValue);
		return nullptr;
	}
	case ElementASTTypes::BinaryOp: {
		return nullptr;
	}
	case ElementASTTypes::UnaryOp: {
		return nullptr;
	}
	case ElementASTTypes::IfStatement: {
		return nullptr;
	}
	case ElementASTTypes::WhileStatement: {
		return nullptr;
	}
	case ElementASTTypes::ForStatement: {
		return nullptr;
	}
	case ElementASTTypes::DoWhileStatement: {
		return nullptr;
	}
	case ElementASTTypes::BreakStatement: {
		return nullptr;
	}
	case ElementASTTypes::ContinueStatement: {
		return nullptr;
	}
	case ElementASTTypes::ExpressionStatement: {
		const ExpressionStatement* node = dynamic_cast<const ExpressionStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		return generate(node->GetExpr());
	}
	case ElementASTTypes::IndexAccess: {
		return nullptr;
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
		return m_Builder->CreateCall(func, args, "calltmp");
	}
	case ElementASTTypes::MemberAccess: {
		return nullptr;
	}
	default:
		// Not implemented!
		return nullptr;
	}
}
