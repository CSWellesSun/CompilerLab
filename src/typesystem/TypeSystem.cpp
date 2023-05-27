#include "typesystem/TypeSystem.h"
#include "common/Defs.h"
#include "lexer/Token.h"
#include "parser/Ast.h"
#include <ostream>
#include <string>
using namespace minisolc;

void TypeSystem::setType(std::string identifier, Type type) {
	auto& map = m_maps.back();
	if (map.find(identifier) == map.end()) {
		// Don't find
		map.insert({identifier, type});
	} else {
		LOG_ERROR("Redefinition!");
	}
}

Type TypeSystem::getType(std::string identifier) {
	for (auto it = m_maps.rbegin(); it != m_maps.rend(); ++it) {
		auto& map = *it;
		if (map.find(identifier) != map.end()) {
			// Find
			return map.at(identifier);
		}
	}

	// Don't find
	LOG_ERROR("Don't Find!");
	return Type::UNKNOWN;
}
Type TypeSystem::analyze(const std::shared_ptr<BaseAST>& AstNode) {
	switch (AstNode->GetASTType()) {
	case ElementASTTypes::SourceUnit: {
		SourceUnit* node = dynamic_cast<SourceUnit*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		for (auto& child: node->getSubNodes()) {
			analyze(child);
		}
		return Type::UNKNOWN;
	}
	case ElementASTTypes::PlainVariableDefinition: {
		PlainVariableDefinition* node = dynamic_cast<PlainVariableDefinition*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Token type = node->GetDeclarationType()->GetType();
		const std::string& name = node->GetName();
		if (type == Token::Int) {
			TypeSystem::setType(name, Type::INTEGER);
		} else if (type == Token::Bool) {
			TypeSystem::setType(name, Type::BOOLEAN);
		} else if (type == Token::String) {
			TypeSystem::setType(name, Type::STRING);
		} else if (type == Token::Float) {
			TypeSystem::setType(name, Type::FLOAT);
		} else if (type == Token::Double) {
			TypeSystem::setType(name, Type::DOUBLE);
		} else {
			LOG_ERROR("Type Error: PlainVariableDefinition.");
			TypeSystem::setType(name, Type::UNKNOWN);
		}
		analyze(node->getVarDefExpr());
		return Type::UNKNOWN;
	}
	case ElementASTTypes::ArrayDefinition: {
		LOG_ERROR("Not Implemented!");
		return Type::UNKNOWN;
	}
	case ElementASTTypes::StructDefinition: {
		LOG_ERROR("Not Implemented!");
		return Type::UNKNOWN;
	}
	case ElementASTTypes::Block: {
		pushMap();
		Block* node = dynamic_cast<Block*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		for (auto& child: node->GetStatements()) {
			analyze(child);
		}
		popMap();
		return Type::UNKNOWN;
	}
	case ElementASTTypes::FunctionDefinition: {
		FunctionDefinition* node = dynamic_cast<FunctionDefinition*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		analyze(node->GetBody());
		std::string name = node->GetName();
		Token type = node->GetDeclarationType()->GetType();
		if (type == Token::Int) {
			TypeSystem::setType(name, Type::INTEGER);
		} else if (type == Token::Bool) {
			TypeSystem::setType(name, Type::BOOLEAN);
		} else if (type == Token::String) {
			TypeSystem::setType(name, Type::STRING);
		} else if (type == Token::Float) {
			TypeSystem::setType(name, Type::FLOAT);
		} else if (type == Token::Double) {
			TypeSystem::setType(name, Type::DOUBLE);
		} else {
			// LOG_ERROR("Type Error: FunctionDefinition.");
			TypeSystem::setType(name, Type::UNKNOWN);
		}
		return Type::UNKNOWN;
	}
	case ElementASTTypes::ReturnStatement: {
		ReturnStatement* node = dynamic_cast<ReturnStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		analyze(node->GetExpr());
		return Type::UNKNOWN;
	}
	case ElementASTTypes::Identifier: {
		Identifier* node = dynamic_cast<Identifier*>(AstNode.get());
		std::string name = node->GetValue();
		Type type = getType(name);
		if (type == Type::UNKNOWN) {
			LOG_ERROR("Type Error: Identifier.");
		}
		node->SetTwoType(type);
		return type;
	}
	case ElementASTTypes::BooleanLiteral: {
		BooleanLiteral* node = dynamic_cast<BooleanLiteral*>(AstNode.get());
		node->SetTwoType(Type::BOOLEAN);
		return Type::BOOLEAN;
	}
	case ElementASTTypes::StringLiteral: {
		StringLiteral* node = dynamic_cast<StringLiteral*>(AstNode.get());
		node->SetTwoType(Type::STRING);
		return Type::STRING;
	}
	case ElementASTTypes::NumberLiteral: {
		NumberLiteral* node = dynamic_cast<NumberLiteral*>(AstNode.get());
		std::string valueString = node->GetValue();
		try {
			if (valueString.find('.') != std::string::npos) {
				node->SetTwoType(Type::DOUBLE);
				return Type::DOUBLE;
			} else {
				node->SetTwoType(Type::INTEGER);
				return Type::INTEGER;
			}
		} catch (std::exception& e) {
			LOG_ERROR("Type Error: Number Literal Error.");
		}
	}
	case ElementASTTypes::Assignment: {
		Assignment* node = dynamic_cast<Assignment*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Type typeLeft = analyze(node->GetLeftHand());
		Type typeRight = analyze(node->GetRightHand());
		if (node->GetLeftHand()->GetASTType() == ElementASTTypes::Identifier) {
			Identifier* leftHand = dynamic_cast<Identifier*>(node->GetLeftHand().get());
			if (typeLeft == Type::UNKNOWN || typeRight == Type::UNKNOWN) {
				LOG_ERROR("Type Error: Assignment.");
				node->SetTwoType(Type::UNKNOWN);
			} else if (typeLeft == typeRight) {
				node->SetTwoType(typeLeft);
			} else {
				if (typeLeft == Type::BOOLEAN) {
					LOG_ERROR("Type Error: Assignment.");
					node->SetTwoType(Type::UNKNOWN);
				} else if (typeLeft == Type::STRING) {
					LOG_ERROR("Type Error: Assignment.");
					node->SetTwoType(Type::UNKNOWN);
				} else if (typeLeft == Type::INTEGER) {
					node->GetRightHand()->SetCastType(Type::INTEGER);
					node->SetTwoType(Type::INTEGER);
				} else if (typeLeft == Type::FLOAT) {
					if (typeRight == Type::INTEGER) {
						node->GetRightHand()->SetCastType(Type::FLOAT);
						node->SetTwoType(Type::FLOAT);
					} else {
						node->GetRightHand()->SetCastType(Type::FLOAT);
						node->SetTwoType(Type::FLOAT);
					}
				} else if (typeLeft == Type::DOUBLE) {
					node->GetRightHand()->SetCastType(Type::DOUBLE);
					node->SetTwoType(Type::DOUBLE);
				}
			}
			return node->GetCastType();
		} else {
			LOG_ERROR("Not Implemented Yet.");
			return Type::UNKNOWN;
			// a[3],a.i……
		}
	}
	case ElementASTTypes::BinaryOp: {
		BinaryOp* node = dynamic_cast<BinaryOp*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Type typeLeft = analyze(node->GetLeftHand());
		Type typeRight = analyze(node->GetRightHand());
		Token op = node->GetOp();
		switch (op) {
		case Token::Comma:
			node->SetTwoType(typeRight);
			break;
		case Token::Or:
			[[fallthrough]];
		case Token::And:
			if (typeLeft == Type::UNKNOWN || typeRight == Type::UNKNOWN) {
				LOG_ERROR("Type Error: BinaryOp Or.");
				node->SetTwoType(Type::UNKNOWN);
			} else if (typeLeft == Type::STRING || typeRight == Type::STRING) {
				LOG_ERROR("Type Error: BinaryOp Or.");
				node->SetTwoType(Type::UNKNOWN);
			} else {
				if (typeLeft != Type::BOOLEAN)
					node->GetLeftHand()->SetCastType(Type::BOOLEAN);
				if (typeRight != Type::BOOLEAN)
					node->GetRightHand()->SetCastType(Type::BOOLEAN);
				node->SetTwoType(Type::BOOLEAN);
			}
			break;
		case Token::BitOr:
			[[fallthrough]];
		case Token::BitXor:
			[[fallthrough]];
		case Token::BitAnd:
			if (typeLeft == Type::STRING || typeLeft == Type::UNKNOWN || typeRight == Type::STRING
				|| typeRight == Type::UNKNOWN) {
				LOG_ERROR("Type Error: BinaryOp BitOr/BitXor/BitAnd.");
				node->SetTwoType(Type::UNKNOWN);
			} else {
				if (typeLeft != Type::INTEGER)
					node->GetLeftHand()->SetCastType(Type::INTEGER);
				if (typeRight != Type::INTEGER)
					node->GetRightHand()->SetCastType(Type::INTEGER);
				node->SetTwoType(Type::INTEGER);
			}
			break;
		case Token::SHL:
			[[fallthrough]];
		case Token::SAR:
			[[fallthrough]];
		case Token::SHR:
			if (typeLeft == Type::INTEGER && typeRight == Type::INTEGER) {
				node->SetTwoType(Type::INTEGER);
			} else {
				LOG_ERROR("Type Error: BinaryOp SHL/SAR/SHR.");
				node->SetTwoType(Type::UNKNOWN);
			}
			break;
		case Token::Add:
			[[fallthrough]];
		case Token::Sub:
			[[fallthrough]];
		case Token::Mul:
			[[fallthrough]];
		case Token::Div:
			if (typeLeft == Type::BOOLEAN || typeLeft == Type::STRING || typeLeft == Type::UNKNOWN
				|| typeRight == Type::BOOLEAN || typeRight == Type::STRING || typeRight == Type::UNKNOWN) {
				LOG_ERROR("Type Error: BinaryOp Add/Sub/Mul/Div.");
				node->SetTwoType(Type::UNKNOWN);
			} else {
				if (typeLeft == Type::INTEGER && typeRight == Type::INTEGER) {
					node->SetTwoType(Type::INTEGER);
				}
				if (typeLeft == Type::INTEGER && (typeRight == Type::FLOAT || typeRight == Type::DOUBLE)) {
					node->GetLeftHand()->SetCastType(typeRight);
					node->SetTwoType(typeRight);
				}
				if ((typeLeft == Type::FLOAT || typeLeft == Type::DOUBLE) && typeRight == Type::INTEGER) {
					node->GetRightHand()->SetCastType(typeLeft);
					node->SetTwoType(typeLeft);
				}
				if (typeLeft == Type::FLOAT && typeRight == Type::DOUBLE) {
					node->GetLeftHand()->SetCastType(Type::DOUBLE);
					node->SetTwoType(Type::DOUBLE);
				}
				if (typeLeft == Type::DOUBLE && typeRight == Type::FLOAT) {
					node->GetRightHand()->SetCastType(Type::DOUBLE);
					node->SetTwoType(Type::DOUBLE);
				}
				if (typeLeft == Type::FLOAT && typeRight == Type::FLOAT)
					node->SetTwoType(Type::FLOAT);
				if (typeLeft == Type::DOUBLE && typeRight == Type::DOUBLE)
					node->SetTwoType(Type::DOUBLE);
			}
			break;
		case Token::Mod:
			if (typeLeft == Type::INTEGER && typeRight == Type::INTEGER) {
				node->SetTwoType(Type::INTEGER);
			} else {
				LOG_ERROR("Type Error: BinaryOp Mod.");
				node->SetTwoType(Type::UNKNOWN);
			}
			break;
		case Token::Exp: // TODO
			LOG_WARNING("Not implemented!");
			break;
		case Token::Equal:
			[[fallthrough]];
		case Token::NotEqual:
			[[fallthrough]];
		case Token::LessThan:
			[[fallthrough]];
		case Token::LessThanOrEqual:
			[[fallthrough]];
		case Token::GreaterThan:
			[[fallthrough]];
		case Token::GreaterThanOrEqual:
			if (typeLeft == Type::UNKNOWN || typeRight == Type::UNKNOWN) {
				LOG_ERROR(
					"Type Error: BinaryOp Equal/NotEqual/LessThan/LessThanOrEqual/GreaterThan/GreaterThanOrEqual.");
				node->SetTwoType(Type::UNKNOWN);
			} else if (typeLeft == Type::STRING && typeRight == Type::STRING) {
				node->SetTwoType(Type::STRING);
			} else if (typeLeft == Type::STRING || typeRight == Type::STRING) {
				LOG_ERROR(
					"Type Error: BinaryOp Equal/NotEqual/LessThan/LessThanOrEqual/GreaterThan/GreaterThanOrEqual.");
				node->SetTwoType(Type::UNKNOWN);
			} else {
				node->SetTwoType(Type::BOOLEAN);
			}
			break;
		default:
			LOG_ERROR("Type Error: BinaryOp.");
			node->SetTwoType(Type::UNKNOWN);
			break;
		}
		return node->GetCastType();
	}
	case ElementASTTypes::UnaryOp: {
		UnaryOp* node = dynamic_cast<UnaryOp*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Type type = analyze(node->GetExpr());
		Token op = node->GetOp();
		switch (op) {
		case Token::Sub:
			[[fallthrough]];
		case Token::Inc:
			[[fallthrough]];
		case Token::Dec:
			if (type == Type::DOUBLE || type == Type::FLOAT || type == Type::INTEGER) {
				node->SetTwoType(type);
			} else {
				LOG_ERROR("Type Error: UnaryOp Sub/Inc/Dec.");
				node->SetTwoType(Type::UNKNOWN);
			}
			break;
		case Token::Not:
			if (type != Type::STRING && type != Type::UNKNOWN) {
				node->SetType(type);
				node->SetCastType(Type::BOOLEAN);
			} else {
				LOG_ERROR("Type Error: UnaryOp Not.");
				node->SetTwoType(Type::UNKNOWN);
			}
		case Token::BitNot:
			if (type == Type::INTEGER) {
				node->SetTwoType(type);
			} else {
				LOG_ERROR("Type Error: UnaryOp BitNot.");
				node->SetTwoType(Type::UNKNOWN);
			}
			break;
		default:
			LOG_ERROR("Unknown UnaryOp");
			node->SetTwoType(Type::UNKNOWN);
		}
		return node->GetCastType();
	}
	case ElementASTTypes::IfStatement: {
		IfStatement* node = dynamic_cast<IfStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Type type = analyze(node->GetCondition());
		if (type != Type::BOOLEAN) {
			LOG_ERROR("Type Error: IfStatement.");
		}
		// for(auto& stmt : node->GetThenStatement()) {
		// 	analyze(stmt);
		// }
		analyze(node->GetThenStatement());
		analyze(node->GetElseStatement());
		return Type::UNKNOWN;
	}
	case ElementASTTypes::WhileStatement: {
		WhileStatement* node = dynamic_cast<WhileStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Type type = analyze(node->GetConditionExpr());
		if (type != Type::BOOLEAN) {
			LOG_ERROR("Type Error: WhileStatement.");
		}
		analyze(node->GetWhileLoopBody());
		return Type::UNKNOWN;
	}
	case ElementASTTypes::ForStatement: {
		ForStatement* node = dynamic_cast<ForStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		analyze(node->GetInitExpr());
		Type type = analyze(node->GetConditionExpr());
		if (type != Type::BOOLEAN) {
			LOG_ERROR("Type Error: ForStatement.");
		}
		analyze(node->GetUpdateExpr());
		analyze(node->GetForLoopBody());
		return Type::UNKNOWN;
	}
	case ElementASTTypes::DoWhileStatement: {
		DoWhileStatement* node = dynamic_cast<DoWhileStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		analyze(node->GetDoWhileLoopBody());
		Type type = analyze(node->GetConditionExpr());
		if (type != Type::BOOLEAN) {
			LOG_ERROR("Type Error: DoWhileStatement.");
		}

		return Type::UNKNOWN;
	}
	case ElementASTTypes::BreakStatement: {
		LOG_INFO("Not Implemented");
		return Type::UNKNOWN;
	}
	case ElementASTTypes::ContinueStatement: {
		LOG_INFO("Not Implemented");
		return Type::UNKNOWN;
	}
	case ElementASTTypes::ExpressionStatement: {
		ExpressionStatement* node = dynamic_cast<ExpressionStatement*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		Type type = analyze(node->GetExpr());
		return Type::UNKNOWN;
	}
	case ElementASTTypes::IndexAccess: {
		IndexAccess* node = dynamic_cast<IndexAccess*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		// Type type = analyze(node->GetExpr());
		// if (type != Type::ARRAY) {
		// 	LOG_ERROR("Type Error: IndexAccess.");
		// }
		Type indexType = analyze(node->GetArrayIndex());
		if (indexType != Type::INTEGER) {
			LOG_ERROR("Type Error: IndexAccess.");
		}
		return Type::UNKNOWN;
	}
	case ElementASTTypes::FunctionCall: {
		FunctionCall* node = dynamic_cast<FunctionCall*>(AstNode.get());
		ASSERT(node != nullptr, "dynamic cast fails.");
		std::string name = node->GetFunctionName();
		Type type = getType(name);
		node->SetTwoType(type);
		// TODO: check args

		// for (auto& arg: node->GetArgs()) {
		// 	analyze(arg);
		// }
		return Type::UNKNOWN;
	}
	case ElementASTTypes::MemberAccess: {
		LOG_INFO("Not Implemented");
		return Type::UNKNOWN;
	}
	default:
		return Type::UNKNOWN;
	}
}
