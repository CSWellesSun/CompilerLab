#pragma once

#include "preprocess/CharStream.h"
#include <cctype>
#include <memory>
#include <string>


namespace minisolc {

/// 完整的solidity语言token，T表示token，K表示keyword
/// TODO: 尽量实现其中的token
#define TOKEN_LIST(T, K)                                                                   \
	/* End of source indicator. */                                                         \
	T(EOS, "EOS", 0)                                                                       \
                                                                                           \
	/* Punctuators (ECMA-262, section 7.7, page 15). */                                    \
	T(LParen, "(", 0)                                                                      \
	T(RParen, ")", 0)                                                                      \
	T(LBrack, "[", 0)                                                                      \
	T(RBrack, "]", 0)                                                                      \
	T(LBrace, "{", 0)                                                                      \
	T(RBrace, "}", 0)                                                                      \
	T(Colon, ":", 0)                                                                       \
	T(Semicolon, ";", 0)                                                                   \
	T(Period, ".", 0)                                                                      \
	T(Conditional, "?", 3)                                                                 \
	T(DoubleArrow, "=>", 0)                                                                \
	T(RightArrow, "->", 0)                                                                 \
                                                                                           \
	/* Assignment operators. */                                                            \
	/* IsAssignmentOp() relies on this block of enum values being */                       \
	/* contiguous and sorted in the same order!*/                                          \
	T(Assign, "=", 2)                                                                      \
	/* The following have to be in exactly the same order as the simple binary operators*/ \
	T(AssignBitOr, "|=", 2)                                                                \
	T(AssignBitXor, "^=", 2)                                                               \
	T(AssignBitAnd, "&=", 2)                                                               \
	T(AssignShl, "<<=", 2)                                                                 \
	T(AssignSar, ">>=", 2)                                                                 \
	T(AssignShr, ">>>=", 2)                                                                \
	T(AssignAdd, "+=", 2)                                                                  \
	T(AssignSub, "-=", 2)                                                                  \
	T(AssignMul, "*=", 2)                                                                  \
	T(AssignDiv, "/=", 2)                                                                  \
	T(AssignMod, "%=", 2)                                                                  \
                                                                                           \
	/* Binary operators sorted by precedence. */                                           \
	/* IsBinaryOp() relies on this block of enum values */                                 \
	/* being contiguous and sorted in the same order! */                                   \
	T(Comma, ",", 1)                                                                       \
	T(Or, "||", 4)                                                                         \
	T(And, "&&", 5)                                                                        \
	T(BitOr, "|", 8)                                                                       \
	T(BitXor, "^", 9)                                                                      \
	T(BitAnd, "&", 10)                                                                     \
	T(SHL, "<<", 11)                                                                       \
	T(SAR, ">>", 11)                                                                       \
	T(SHR, ">>>", 11)                                                                      \
	T(Add, "+", 12)                                                                        \
	T(Sub, "-", 12)                                                                        \
	T(Mul, "*", 13)                                                                        \
	T(Div, "/", 13)                                                                        \
	T(Mod, "%", 13)                                                                        \
	T(Exp, "**", 14)                                                                       \
                                                                                           \
	/* Compare operators sorted by precedence. */                                          \
	/* IsCompareOp() relies on this block of enum values */                                \
	/* being contiguous and sorted in the same order! */                                   \
	T(Equal, "==", 6)                                                                      \
	T(NotEqual, "!=", 6)                                                                   \
	T(LessThan, "<", 7)                                                                    \
	T(GreaterThan, ">", 7)                                                                 \
	T(LessThanOrEqual, "<=", 7)                                                            \
	T(GreaterThanOrEqual, ">=", 7)                                                         \
                                                                                           \
	/* Unary operators. */                                                                 \
	/* IsUnaryOp() relies on this block of enum values */                                  \
	/* being contiguous and sorted in the same order! */                                   \
	T(Not, "!", 0)                                                                         \
	T(BitNot, "~", 0)                                                                      \
	T(Inc, "++", 0)                                                                        \
	T(Dec, "--", 0)                                                                        \
	K(Delete, "delete", 0)                                                                 \
                                                                                           \
	/* Inline Assembly Operators */                                                        \
	T(AssemblyAssign, ":=", 2)                                                             \
	/* Keywords */                                                                         \
	K(Abstract, "abstract", 0)                                                             \
	K(Anonymous, "anonymous", 0)                                                           \
	K(As, "as", 0)                                                                         \
	K(Assembly, "assembly", 0)                                                             \
	K(Break, "break", 0)                                                                   \
	K(Catch, "catch", 0)                                                                   \
	K(Constant, "constant", 0)                                                             \
	K(Constructor, "constructor", 0)                                                       \
	K(Continue, "continue", 0)                                                             \
	K(Contract, "contract", 0)                                                             \
	K(Do, "do", 0)                                                                         \
	K(Else, "else", 0)                                                                     \
	K(Enum, "enum", 0)                                                                     \
	K(Emit, "emit", 0)                                                                     \
	K(Event, "event", 0)                                                                   \
	K(External, "external", 0)                                                             \
	K(Fallback, "fallback", 0)                                                             \
	K(For, "for", 0)                                                                       \
	K(Function, "function", 0)                                                             \
	K(Hex, "hex", 0)                                                                       \
	K(If, "if", 0)                                                                         \
	K(Indexed, "indexed", 0)                                                               \
	K(Interface, "interface", 0)                                                           \
	K(Internal, "internal", 0)                                                             \
	K(Immutable, "immutable", 0)                                                           \
	K(Import, "import", 0)                                                                 \
	K(Is, "is", 0)                                                                         \
	K(Library, "library", 0)                                                               \
	K(Mapping, "mapping", 0)                                                               \
	K(Memory, "memory", 0)                                                                 \
	K(Modifier, "modifier", 0)                                                             \
	K(New, "new", 0)                                                                       \
	K(Override, "override", 0)                                                             \
	K(Payable, "payable", 0)                                                               \
	K(Public, "public", 0)                                                                 \
	K(Pragma, "pragma", 0)                                                                 \
	K(Private, "private", 0)                                                               \
	K(Pure, "pure", 0)                                                                     \
	K(Receive, "receive", 0)                                                               \
	K(Return, "return", 0)                                                                 \
	K(Returns, "returns", 0)                                                               \
	K(Storage, "storage", 0)                                                               \
	K(CallData, "calldata", 0)                                                             \
	K(Struct, "struct", 0)                                                                 \
	K(Throw, "throw", 0)                                                                   \
	K(Try, "try", 0)                                                                       \
	K(Type, "type", 0)                                                                     \
	K(Unchecked, "unchecked", 0)                                                           \
	K(Unicode, "unicode", 0)                                                               \
	K(Using, "using", 0)                                                                   \
	K(View, "view", 0)                                                                     \
	K(Virtual, "virtual", 0)                                                               \
	K(While, "while", 0)                                                                   \
                                                                                           \
	/* type keywords*/                                                                     \
	K(Int, "int", 0)                                                                       \
	K(UInt, "uint", 0)                                                                     \
	K(String, "string", 0)                                                                 \
	K(Bool, "bool", 0)                                                                     \
	K(Float, "float", 0)                                                                   \
	K(Double, "double", 0)                                                                 \
	K(Void, "void", 0)                                                                     \
	T(IntM, "intM", 0)                                                                     \
	T(UIntM, "uintM", 0)                                                                   \
	T(TypesEnd, nullptr, 0) /* used as type enum end marker */                             \
                                                                                           \
	/* Literals */                                                                         \
	K(TrueLiteral, "true", 0)                                                              \
	K(FalseLiteral, "false", 0)                                                            \
	T(IntNumber, nullptr, 0)                                                               \
	T(DoubleNumber, nullptr, 0)                                                            \
	T(StringLiteral, nullptr, 0)                                                           \
	T(UnicodeStringLiteral, nullptr, 0)                                                    \
	T(HexStringLiteral, nullptr, 0)                                                        \
	T(CommentLiteral, nullptr, 0)                                                          \
                                                                                           \
	/* Identifiers (not keywords or future reserved words). */                             \
	T(Identifier, nullptr, 0)                                                              \
                                                                                           \
	/* Keywords reserved for future use. */                                                \
	K(After, "after", 0)                                                                   \
	K(Alias, "alias", 0)                                                                   \
	K(Apply, "apply", 0)                                                                   \
	K(Auto, "auto", 0)                                                                     \
	K(Byte, "byte", 0)                                                                     \
	K(Case, "case", 0)                                                                     \
	K(CopyOf, "copyof", 0)                                                                 \
	K(Default, "default", 0)                                                               \
	K(Define, "define", 0)                                                                 \
	K(Final, "final", 0)                                                                   \
	K(Implements, "implements", 0)                                                         \
	K(In, "in", 0)                                                                         \
	K(Inline, "inline", 0)                                                                 \
	K(Let, "let", 0)                                                                       \
	K(Macro, "macro", 0)                                                                   \
	K(Match, "match", 0)                                                                   \
	K(Mutable, "mutable", 0)                                                               \
	K(NullLiteral, "null", 0)                                                              \
	K(Of, "of", 0)                                                                         \
	K(Partial, "partial", 0)                                                               \
	K(Promise, "promise", 0)                                                               \
	K(Reference, "reference", 0)                                                           \
	K(Relocatable, "relocatable", 0)                                                       \
	K(Sealed, "sealed", 0)                                                                 \
	K(Sizeof, "sizeof", 0)                                                                 \
	K(Static, "static", 0)                                                                 \
	K(Supports, "supports", 0)                                                             \
	K(Switch, "switch", 0)                                                                 \
	K(Typedef, "typedef", 0)                                                               \
	K(TypeOf, "typeof", 0)                                                                 \
	K(Var, "var", 0)                                                                       \
                                                                                           \
	/* Yul-specific tokens, but not keywords. */                                           \
	T(Leave, "leave", 0)                                                                   \
                                                                                           \
	/* Illegal token - not able to scan. */                                                \
	T(Illegal, "ILLEGAL", 0)                                                               \
                                                                                           \
	/* Scanner-internal use only. */                                                       \
	T(Whitespace, nullptr, 0)


/// Token
enum class Token : unsigned int {
#define T(name, string, precedence) name,
	TOKEN_LIST(T, T) NUM_TOKENS
#undef T

};

enum class StateMutability {
	Nonpayable,
	Payable,
	View,
	Pure,
};

enum class Visibility { Default, Private, Internal, Public, External };

Token keywordByName(std::string _name);
char const* tokenToString(Token tok);

StateMutability stateMutabilityByName(std::string _name);
char const* stateMutabilityToString(StateMutability _state);

Visibility visibilityByName(std::string _name);
char const* visibilityToString(Visibility _visibility);

constexpr bool isType(Token tok) { return tok >= Token::Int && tok < Token::TypesEnd; }
constexpr bool isLiteral(Token tok) { return tok >= Token::TrueLiteral && tok <= Token::CommentLiteral; }
constexpr bool isAssignmentOp(Token tok) { return tok >= Token::Assign && tok <= Token::AssignMod; }
constexpr bool isBinaryOp(Token tok) { return tok >= Token::Comma && tok <= Token::Exp; }
constexpr bool isUnaryOp(Token tok) { return (tok >= Token::Not && tok <= Token::Delete) || tok == Token::Sub; }
constexpr bool isCompareOp(Token tok) { return tok >= Token::Equal && tok <= Token::GreaterThanOrEqual; }

constexpr bool isVisibility(Token tok) {
	return tok == Token::Private || tok == Token::Internal || tok == Token::Public || tok == Token::External;
}

constexpr bool isalus(char c) { return std::isalpha(c) || c == '_'; };
constexpr bool isalnumus(char c) { return std::isalpha(c) || std::isdigit(c) || c == '_'; };
constexpr bool isoct(char c) { return c >= '0' && c <= '7'; };
constexpr bool issep(char c) { return !std::isalnum(c) && c != '.'; }

constexpr int precedence(Token tok) {
	int constexpr precs[static_cast<size_t>(Token::NUM_TOKENS)] = {
#define T(name, string, precedence) precedence,
		TOKEN_LIST(T, T)
#undef T
	};
	return precs[static_cast<size_t>(tok)];
}

struct Location {
	std::shared_ptr<Line> m_line;
	size_t m_start;
	size_t m_end;

	Location(std::shared_ptr<Line> line, size_t start, size_t end): m_line(line), m_start(start), m_end(end){};
};

struct TokenInfo {
	Token m_tok;
	std::string m_val;
	Location m_loc;

	TokenInfo(const Token tok, const std::string& v, std::shared_ptr<Line> line, size_t start, size_t end)
		: m_tok(tok), m_val(v), m_loc(line, start, end){};
	TokenInfo(const Token tok, std::string&& v, std::shared_ptr<Line> line, size_t start, size_t end)
		: m_tok(tok), m_val(v), m_loc(line, start, end){};
};

enum class Type {
	UNKNOWN,
	INTEGER,
	DOUBLE,
	FLOAT,
	STRING,
	BOOLEAN,
	//... and other types if needed.
};

Type typeByName(std::string _name);
char const* typeToString(Type type);
}