#include "codegen/CodeGen.h"
#include "common/Defs.h"
#include "parser/Ast.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <memory>
#include <fstream>

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
		return llvm::Type::getInt8PtrTy(*m_Context);
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

llvm::Value* CodeGenerator::generate(const std::shared_ptr<BaseAST>& AstNode, bool beginBlock, bool isleftval) {
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
		const PlainVariableDefinition* node = dynamic_cast<const PlainVariableDefinition*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Token type = node->GetDeclarationType()->GetType();
		llvm::Type* llvmType = getLLVMType(type);
		llvm::Value* res = m_Builder->CreateAlloca(llvmType, nullptr);
		setSymbolValue(node->GetName(), res);
		setSymbolType(node->GetName(), llvmType);

		auto& expr = node->getVarDefExpr();
		if (expr != nullptr) {
			res = generate(
				std::make_shared<Assignment>(std::make_shared<Identifier>(node->GetName()), Token::Assign, expr));
		} else {
			// initialize the variable, maybe change later
			llvm::Value* value = getInitValue(type);
			if (value != nullptr)
				m_Builder->CreateStore(value, res);
		}

		return res;
	}
	case ElementASTTypes::ArrayDefinition: {
		const ArrayDefinition* node = dynamic_cast<const ArrayDefinition*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		llvm::Type* arrType = getLLVMType(node->GetDeclarationType()->GetType());
		llvm::Value* arrSize = generate(node->GetArraySize());
		llvm::Value* res = m_Builder->CreateAlloca(arrType, arrSize);
		setSymbolValue(node->GetName(), res);
		setSymbolType(node->GetName(), arrType);
		return res;
	}
	case ElementASTTypes::StructDefinition: {
		LOG_WARNING("Not implemented!");
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
		const auto& paralist = node->GetParameterList();
		const auto& argsvt
			= (paralist != nullptr) ? paralist->GetArgs() : std::vector<std::shared_ptr<VariableDefinition>>{};
		for (const auto& arg: argsvt) {
			argTypes.push_back(getLLVMType(arg->GetDeclarationType()->GetType()));
		}
		llvm::FunctionType* funcType
			= llvm::FunctionType::get(getLLVMType(node->GetDeclarationType()->GetType()), argTypes, false);
		func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->GetName(), m_Module.get());
		// Set names for all arguments
		unsigned idx = 0;
		for (auto& arg: func->args()) {
			arg.setName(argsvt[idx++]->GetName());
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
		std::string valueString = dynamic_cast<const StringLiteral*>(AstNode.get())->GetValue();
		valueString = valueString.substr(1, valueString.size() - 2); // remove ""
		size_t stridx;

		// escape character
		if ((stridx = valueString.find("\\n")) != std::string::npos) {
			valueString.replace(stridx, 2, "\n");
		}
		if ((stridx = valueString.find("\\r")) != std::string::npos) {
			valueString.replace(stridx, 2, "\r");
		}
		if ((stridx = valueString.find("\\t")) != std::string::npos) {
			valueString.replace(stridx, 2, "\t");
		}

		auto res = m_Builder->CreateGlobalStringPtr(llvm::StringRef(valueString));
		return res;
	}
	case ElementASTTypes::NumberLiteral: {
		/// TODO: check its type
		std::string valueString = dynamic_cast<const NumberLiteral*>(AstNode.get())->GetValue();
		llvm::Constant* res;
		try {
			if (valueString.find('.') != std::string::npos) {
				/* double */
				double value = std::stod(valueString);
				res = llvm::ConstantFP::get(m_Builder->getDoubleTy(), value);
			} else {
				/* integer */
				uint32_t value = static_cast<uint32_t>(std::stoi(valueString));
				res = m_Builder->getInt32(value);
			}
		} catch (std::exception& e) {
			LOG_ERROR("Number Literial fails, %s", e.what());
		}
		
		return res;
	}
	case ElementASTTypes::Assignment: {
		const Assignment* node = dynamic_cast<const Assignment*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		llvm::Value  *leftHandValue, *rightHandValue;
		if (node->GetLeftHand()->GetASTType() == ElementASTTypes::Identifier) {
			const Identifier* leftHand = dynamic_cast<const Identifier*>(node->GetLeftHand().get());
			ASSERT(leftHand != nullptr, "dynamic cast fails.");
			leftHandValue = getSymbolValue(leftHand->GetValue());
			rightHandValue = generate(node->GetRightHand());
		} else if (node->GetLeftHand()->GetASTType() == ElementASTTypes::IndexAccess) {
			leftHandValue = generate(node->GetLeftHand(), true, true);
			rightHandValue = generate(node->GetRightHand());
		} else {
			LOG_ERROR("Invalid type in assignment.");
			return nullptr;
		}
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
		llvm::Value* res;
		if (leftHandValue->getType()->isFloatingPointTy() || rightHandValue->getType()->isFloatingPointTy() ) {
			/* float point */
			switch (op) {
			case Token::Comma:
				res =  rightHandValue; break;
			case Token::Or ... Token::SHR:
				LOG_ERROR("Invalid operator for float points!");
				res = nullptr; break;
			case Token::Add:
				res = m_Builder->CreateFAdd(leftHandValue, rightHandValue); break;
			case Token::Sub:
				res = m_Builder->CreateFSub(leftHandValue, rightHandValue); break;
			case Token::Mul:
				res = m_Builder->CreateFMul(leftHandValue, rightHandValue); break;
			case Token::Div:
				res = m_Builder->CreateFDiv(leftHandValue, rightHandValue); break;
			case Token::Mod:
				res = m_Builder->CreateFRem(leftHandValue, rightHandValue); break; // ? 
			case Token::Exp: // TODO
				LOG_WARNING("Not implemented!");
				res = nullptr; break;
			case Token::Equal:
				res = m_Builder->CreateFCmpUEQ(leftHandValue, rightHandValue); break;
			case Token::NotEqual:
				res = m_Builder->CreateFCmpUNE(leftHandValue, rightHandValue); break;
			case Token::LessThan:
				res = m_Builder->CreateFCmpULT(leftHandValue, rightHandValue); break;
			case Token::LessThanOrEqual:
				res = m_Builder->CreateFCmpULE(leftHandValue, rightHandValue); break;
			case Token::GreaterThan:
				res = m_Builder->CreateFCmpUGT(leftHandValue, rightHandValue); break;
			case Token::GreaterThanOrEqual:
				res = m_Builder->CreateFCmpUGE(leftHandValue, rightHandValue); break;
			default:
				res = nullptr;
			}
		} else {
			/* integer */
			switch (op) {
			case Token::Comma:
				res = rightHandValue; break;
			case Token::Or: // 考虑短路
				res = m_Builder->CreateLogicalOr(leftHandValue, rightHandValue); break;
			case Token::And:
				res = m_Builder->CreateLogicalAnd(leftHandValue, rightHandValue); break;
			case Token::BitOr:
				res = m_Builder->CreateOr(leftHandValue, rightHandValue); break;
			case Token::BitXor:
				res = m_Builder->CreateXor(leftHandValue, rightHandValue); break;
			case Token::BitAnd:
				res = m_Builder->CreateAnd(leftHandValue, rightHandValue); break;
			case Token::SHL:
				res = m_Builder->CreateShl(leftHandValue, rightHandValue); break;
			case Token::SAR:
				res = m_Builder->CreateAShr(leftHandValue, rightHandValue); break;
			case Token::SHR:
				res = m_Builder->CreateLShr(leftHandValue, rightHandValue); break;
			case Token::Add:
				res = m_Builder->CreateAdd(leftHandValue, rightHandValue); break;
			case Token::Sub:
				res = m_Builder->CreateSub(leftHandValue, rightHandValue); break;
			case Token::Mul:
				res = m_Builder->CreateMul(leftHandValue, rightHandValue); break;
			case Token::Div:
				res = m_Builder->CreateUDiv(leftHandValue, rightHandValue); break;
			case Token::Mod:
				res = m_Builder->CreateURem(leftHandValue, rightHandValue);  break;
			case Token::Exp: // TODO
				LOG_WARNING("Not implemented!");
				res = nullptr; break;
			case Token::Equal:
				res = m_Builder->CreateICmpEQ(leftHandValue, rightHandValue); break;
			case Token::NotEqual:
				res = m_Builder->CreateICmpNE(leftHandValue, rightHandValue); break;
			case Token::LessThan:
				res = m_Builder->CreateICmpULT(leftHandValue, rightHandValue); break;
			case Token::LessThanOrEqual:
				res = m_Builder->CreateICmpULE(leftHandValue, rightHandValue); break;
			case Token::GreaterThan:
				res = m_Builder->CreateICmpUGT(leftHandValue, rightHandValue); break;
			case Token::GreaterThanOrEqual:
				res = m_Builder->CreateICmpUGE(leftHandValue, rightHandValue); break;
			default:
				res = nullptr;
		}
		} // if
		if (res == nullptr) {
			LOG_WARNING("Arithmetic operation fails! Code = %d", static_cast<int>(op));
		}

		return res;
	}
	case ElementASTTypes::UnaryOp: {
		const UnaryOp* node = dynamic_cast<const UnaryOp*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Token op = node->GetOp();
		llvm::Value* value = generate(node->GetExpr());
		bool is_prefix = node->IsPrefix();
		llvm::Value* res;

		if (value->getType()->isFloatingPointTy()) {
			switch (op) {
			case Token::Sub:
				res = m_Builder->CreateFNeg(value); break;
			case Token::Not:
				res = m_Builder->CreateNot(value); break;
			case Token::BitNot:
				res = m_Builder->CreateNot(value); break; // ?
			case Token::Inc: {
				const Identifier* id = dynamic_cast<const Identifier*>(node->GetExpr().get());
				ASSERT(id != nullptr, "dynamic cast fails.");
				llvm::Value* temp = m_Builder->CreateFAdd(value, llvm::ConstantFP::get(m_Builder->getDoubleTy(), 1.0));
				m_Builder->CreateStore(temp, getSymbolValue(id->GetValue()));
				res = is_prefix ? temp : value;
				break;
			}
			case Token::Dec: {
				const Identifier* id = dynamic_cast<const Identifier*>(node->GetExpr().get());
				ASSERT(id != nullptr, "dynamic cast fails.");
				llvm::Value* temp = m_Builder->CreateFSub(value, llvm::ConstantFP::get(m_Builder->getDoubleTy(), 1.0));
				m_Builder->CreateStore(temp, getSymbolValue(id->GetValue()));
				res = is_prefix ? temp : value;
				break;
			}
			default:
				res = nullptr;
			}

		} else {
			switch (op) {
			case Token::Sub:
				res = m_Builder->CreateNeg(value); break;
			case Token::Not:
				res = m_Builder->CreateNot(value); break;
			case Token::BitNot:
				res = m_Builder->CreateNot(value); break;
			case Token::Inc: {
				const Identifier* id = dynamic_cast<const Identifier*>(node->GetExpr().get());
				ASSERT(id != nullptr, "dynamic cast fails.");
				llvm::Value* temp = m_Builder->CreateAdd(value, m_Builder->getInt32(1));
				m_Builder->CreateStore(temp, getSymbolValue(id->GetValue()));
				res = is_prefix ? temp : value;
				break;
			}
			case Token::Dec: {
				const Identifier* id = dynamic_cast<const Identifier*>(node->GetExpr().get());
				ASSERT(id != nullptr, "dynamic cast fails.");
				llvm::Value* temp = m_Builder->CreateSub(value, m_Builder->getInt32(1));
				m_Builder->CreateStore(temp, getSymbolValue(id->GetValue()));
				res = is_prefix ? temp : value;
				break;
			}
			default:
				res = nullptr;
			}
		} // if
		if (res == nullptr) {
			LOG_WARNING("Arithmetic operation fails! Code = %d", static_cast<int>(op));
		} 
		return res;
	}
	case ElementASTTypes::IfStatement: {
		const IfStatement* node = dynamic_cast<const IfStatement*>(AstNode.get());
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
		const WhileStatement* node = dynamic_cast<const WhileStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		llvm::Function* function = m_Builder->GetInsertBlock()->getParent();
		llvm::BasicBlock* block = llvm::BasicBlock::Create(*m_Context);
		llvm::BasicBlock* after = llvm::BasicBlock::Create(*m_Context);

		llvm::Value* condition = generate(node->GetConditionExpr());

		// according to parser, condition won't be nullptr
		/*
			if (condition == nullptr) {
				LOG_INFO("Maybe an invalid while loop condition.");
				return nullptr;
			}
		*/
		m_Builder->CreateCondBr(condition, block, after);
		m_Builder->SetInsertPoint(block);
		function->getBasicBlockList().push_back(block);

		pushBlock();
		generate(node->GetWhileLoopBody());
		popBlock();

		condition = generate(node->GetConditionExpr());

		m_Builder->CreateCondBr(condition, block, after);

		m_Builder->SetInsertPoint(after);
		function->getBasicBlockList().push_back(after);

		return nullptr;
	}
	case ElementASTTypes::ForStatement: {
		const ForStatement* node = dynamic_cast<const ForStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		llvm::Function* function = m_Builder->GetInsertBlock()->getParent();
		llvm::BasicBlock* block = llvm::BasicBlock::Create(*m_Context);
		llvm::BasicBlock* after = llvm::BasicBlock::Create(*m_Context);

		if (node->GetInitExpr() != nullptr) {
			generate(node->GetInitExpr());
		}

		// We assume no empty condition, such as for (;;)
		llvm::Value* condition = generate(node->GetConditionExpr());

		// according to parser, condition won't be nullptr
		/*
			if (condition == nullptr) {
				LOG_INFO("Maybe an invalid for loop condition.");
				return nullptr;
			}
		*/
		m_Builder->CreateCondBr(condition, block, after);
		m_Builder->SetInsertPoint(block);
		function->getBasicBlockList().push_back(block);
		pushBlock();
		generate(node->GetForLoopBody());
		popBlock();

		if (node->GetUpdateExpr() != nullptr) {
			generate(node->GetUpdateExpr());
		}

		condition = generate(node->GetConditionExpr());

		m_Builder->CreateCondBr(condition, block, after);

		function->getBasicBlockList().push_back(after);
		m_Builder->SetInsertPoint(after);

		return nullptr;
	}
	case ElementASTTypes::DoWhileStatement: {
		const DoWhileStatement* node = dynamic_cast<const DoWhileStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		llvm::Function* function = m_Builder->GetInsertBlock()->getParent();
		llvm::BasicBlock* block = llvm::BasicBlock::Create(*m_Context);
		llvm::BasicBlock* after = llvm::BasicBlock::Create(*m_Context);
		m_Builder->SetInsertPoint(block);
		function->getBasicBlockList().push_back(block);
		pushBlock();
		generate(node->GetDoWhileLoopBody());
		popBlock();

		llvm::Value* condition = generate(node->GetConditionExpr());
		m_Builder->CreateCondBr(condition, block, after);
		function->getBasicBlockList().push_back(after);
		m_Builder->SetInsertPoint(after);
		return nullptr;
	}
	case ElementASTTypes::BreakStatement: {
		LOG_WARNING("Not implemented");
		return nullptr;
	}
	case ElementASTTypes::ContinueStatement: {
		LOG_WARNING("Not implemented");
		return nullptr;
	}
	case ElementASTTypes::ExpressionStatement: {
		const ExpressionStatement* node = dynamic_cast<const ExpressionStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		return generate(node->GetExpr());
	}
	case ElementASTTypes::IndexAccess: {
		IndexAccess* node = dynamic_cast<IndexAccess*>(AstNode.get());
		const auto arrIdentifier = std::dynamic_pointer_cast<Identifier>(node->GetArrayName());
		const std::string& arrName = arrIdentifier->GetValue();
		auto varptr = this->getSymbolValue(arrName);
		llvm::Type* type = this->getSymbolType(arrName);
		// auto arrSize = this->getArraySize(arrName);
		llvm::Value* arrIdx = generate(node->GetArrayIndex());

		auto ptr = m_Builder->CreateInBoundsGEP(type, varptr, arrIdx);
		
		auto res = m_Builder->CreateAlignedLoad(type, ptr, llvm::MaybeAlign(4ull));
		/*
			When IndexAccess is a left value, for example,
				a[1] = 10;
			we return a pointer to a[1], allowing us to write the address.
			When IndexAccess is a right value, for example,
				i = a[0];
			we return the value of a[0].

			This may not be elegant, hope to improve it in the future.
		*/
		return isleftval ? ptr : res;
	}
	case ElementASTTypes::FunctionCall: {
		const FunctionCall* node = dynamic_cast<const FunctionCall*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		const std::string& funcName = node->GetFunctionName();
		llvm::Function* func = m_Module->getFunction(funcName);
		if (func == nullptr) {
			LOG_ERROR("Function %s not found.", funcName.c_str());
			return nullptr;
		}

		if (func->arg_size() != node->GetArgs().size() && func->isVarArg() == false) {
			LOG_ERROR("Function %s argument size mismatch.", funcName.c_str());
			return nullptr;
		}

		std::vector<llvm::Value*> args;
		for (const auto& arg: node->GetArgs()) {
			args.push_back(generate(arg));
			if (args.back() == nullptr) {
				LOG_ERROR("Function %s argument generation failed.", funcName.c_str());
				return nullptr;
			}
		}
		return m_Builder->CreateCall(func, args);
	}
	case ElementASTTypes::MemberAccess: {
		LOG_WARNING("Not implemented");
		return nullptr;
	}
	default:
		// Not implemented!
		return nullptr;
	} // switch
}

void CodeGenerator::createSyscall() {
	using namespace std::literals;
	/* scanf */
	llvm::FunctionType* scanf_type = llvm::FunctionType::get(m_Builder->getInt32Ty(), true);
	llvm::Function* scanf_func = llvm::Function::Create(scanf_type, llvm::Function::ExternalLinkage, llvm::Twine("scanf"), m_Module.get());
	scanf_func->setCallingConv(llvm::CallingConv::C);
	m_syscalls.emplace("scanf"s, std::move(scanf_func));

	/* printf */
	std::vector<llvm::Type*> arg_types;
	arg_types.push_back(m_Builder->getInt8PtrTy());
	auto printf_type = llvm::FunctionType::get(m_Builder->getInt32Ty(), llvm::makeArrayRef(arg_types), true);
	auto printf_func = llvm::Function::Create(printf_type, llvm::Function::ExternalLinkage, llvm::Twine("printf"), m_Module.get());
	printf_func->setCallingConv(llvm::CallingConv::C);
	m_syscalls.emplace("printf"s, std::move(printf_func));
	
	return;
}


void CodeGenerator::srctollFile(const std::string& srcfilename) const {
	size_t stridx = srcfilename.rfind(".");
	if (srcfilename.substr(stridx + 1) != "sol") {
		// The suffix name should be .sol
		LOG_WARNING("Maybe invalid source file.");
	}
	// Change the suffix name to .ll
	std::string desfilename = srcfilename.substr(0, stridx + 1) + "ll";

	std::error_code ec;
	llvm::raw_fd_stream ofs {llvm::StringRef(desfilename), ec};
	m_Module->print(ofs, nullptr);

	if (bool(ec)) {
		LOG_WARNING("to .ll file fails. %s", ec.message().c_str());
	}
	return;
}
