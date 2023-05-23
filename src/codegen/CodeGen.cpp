#include "codegen/CodeGen.h"
#include "common/Defs.h"
#include "parser/Ast.h"

#include <llvm-14/llvm/IR/Constants.h>
#include <llvm-14/llvm/IR/Function.h>
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
		llvm::Value* res = m_Builder->CreateAlloca(type, nullptr);
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
		if (assignmentOp == Token::Assign) {
			return m_Builder->CreateStore(rightHandValue, leftHandValue);
		}
		return nullptr;
	}
	case ElementASTTypes::BinaryOp: {
		const BinaryOp* node = dynamic_cast<const BinaryOp*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Token op = node->GetOp();
		llvm::Value* leftHandValue = generate(node->GetLeftHand());
		llvm::Value* rightHandValue = generate(node->GetRightHand());

		switch (op) {
		case Token::Comma:
			return rightHandValue;
		case Token::Or: // 考虑短路
			return m_Builder->CreateLogicalOr(leftHandValue, rightHandValue);
		case Token::And:
			return m_Builder->CreateLogicalAnd(leftHandValue, rightHandValue);
		case Token::BitOr:
			return m_Builder->CreateOr(leftHandValue, rightHandValue);
		case Token::BitXor:
			return m_Builder->CreateXor(leftHandValue, rightHandValue);
		case Token::BitAnd:
			return m_Builder->CreateAnd(leftHandValue, rightHandValue);
		case Token::SHL:
			return m_Builder->CreateShl(leftHandValue, rightHandValue);
		case Token::SAR:
			return m_Builder->CreateAShr(leftHandValue, rightHandValue);
		case Token::SHR:
			return m_Builder->CreateLShr(leftHandValue, rightHandValue);
		case Token::Add:
			return m_Builder->CreateFAdd(leftHandValue, rightHandValue);
		case Token::Sub:
			return m_Builder->CreateFSub(leftHandValue, rightHandValue);
		case Token::Mul:
			return m_Builder->CreateFMul(leftHandValue, rightHandValue);
		case Token::Div:
			return m_Builder->CreateFDiv(leftHandValue, rightHandValue);
		case Token::Mod:
			return m_Builder->CreateFRem(leftHandValue, rightHandValue);
		case Token::Exp: // TODO
			return nullptr;
		case Token::Equal:
			return m_Builder->CreateFCmpUEQ(leftHandValue, rightHandValue);
		case Token::NotEqual:
			return m_Builder->CreateFCmpUNE(leftHandValue, rightHandValue);
		case Token::LessThan:
			return m_Builder->CreateFCmpULT(leftHandValue, rightHandValue);
		case Token::LessThanOrEqual:
			return m_Builder->CreateFCmpULE(leftHandValue, rightHandValue);
		case Token::GreaterThan:
			return m_Builder->CreateFCmpUGT(leftHandValue, rightHandValue);
		case Token::GreaterThanOrEqual:
			return m_Builder->CreateFCmpUGE(leftHandValue, rightHandValue);
		default:
			return nullptr;
		}
		return nullptr;
	}
	case ElementASTTypes::UnaryOp: {
		const UnaryOp* node = dynamic_cast<const UnaryOp*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Token op = node->GetOp();
		llvm::Value* value = generate(node->GetExpr());
		bool is_prefix = node->IsPrefix();

		switch (op) {
		case Token::Sub:
			return m_Builder->CreateFNeg(value);
		case Token::Not:
			return m_Builder->CreateNot(value);
		case Token::BitNot:
			return m_Builder->CreateNot(value);
		case Token::Inc: {
			const Identifier* id = dynamic_cast<const Identifier*>(node->GetExpr().get());
			ASSERT(id != nullptr, "dynamic cast fails.");
			llvm::Value* temp = m_Builder->CreateFAdd(value, llvm::ConstantFP::get(m_Builder->getDoubleTy(), 1.0));
			m_Builder->CreateStore(temp, getSymbolValue(id->GetValue()));
			return is_prefix ? temp : value;
		}
		case Token::Dec: {
			const Identifier* id = dynamic_cast<const Identifier*>(node->GetExpr().get());
			ASSERT(id != nullptr, "dynamic cast fails.");
			llvm::Value* temp = m_Builder->CreateFSub(value, llvm::ConstantFP::get(m_Builder->getDoubleTy(), 1.0));
			m_Builder->CreateStore(temp, getSymbolValue(id->GetValue()));
			return is_prefix ? temp : value;
		}
		default:
			return nullptr;
		}
		return nullptr;
	}
	case ElementASTTypes::IfStatement: {
		IfStatement* node = dynamic_cast<IfStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		llvm::Value* condition = generate(node->GetCondition());
		llvm::Function* function = m_Builder->GetInsertBlock()->getParent();
		llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(*m_Context);
		llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(*m_Context);
		llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*m_Context);
		if (node->GetThenStatement() == nullptr) {
			thenBlock = mergeBlock;
		}
		if (node->GetElseStatement() == nullptr) {
			elseBlock = mergeBlock;
		}
		if (thenBlock == elseBlock) {
			return nullptr;
		}
		m_Builder->CreateCondBr(condition, thenBlock, elseBlock);

		if (node->GetThenStatement() != nullptr) {
			function->getBasicBlockList().push_back(thenBlock);
			m_Builder->SetInsertPoint(thenBlock);
			llvm::Value* thenValue = generate(node->GetThenStatement());
			if ((thenBlock = m_Builder->GetInsertBlock())->getTerminator() == nullptr) {
				m_Builder->CreateBr(mergeBlock);
			}
		}

		if (node->GetElseStatement() != nullptr) {
			function->getBasicBlockList().push_back(elseBlock);
			m_Builder->SetInsertPoint(elseBlock);
			llvm::Value* elseValue = generate(node->GetElseStatement());
			if ((elseBlock = m_Builder->GetInsertBlock())->getTerminator() == nullptr) {
				m_Builder->CreateBr(mergeBlock);
			}
		}

		function->getBasicBlockList().push_back(mergeBlock);
		m_Builder->SetInsertPoint(mergeBlock);
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
		return m_Builder->CreateCall(func, args);
	}
	case ElementASTTypes::MemberAccess: {
		return nullptr;
	}
	default:
		// Not implemented!
		return nullptr;
	}
}