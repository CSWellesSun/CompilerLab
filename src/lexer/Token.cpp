#include "lexer/Token.h"
#include <map>
#include <string>

using namespace minisolc;

Token minisolc::keywordByName(std::string _name) {
#define KEYWORD(name, string, precedence) {string, Token::name},
#define TOKEN(name, string, precedence)
	static std::map<std::string, Token> const keywords({TOKEN_LIST(TOKEN, KEYWORD)});
#undef KEYWORD
#undef TOKEN
	// 需要考虑int8 | int16 | int32 | int64 | int128 | int256等特殊情况
	// uint8 | uint16 | uint32 | uint64 | uint128 | uint256等特殊情况
	if (_name == "int8" || _name == "int16" || _name == "int32" || _name == "int64" || _name == "int128" || _name == "int256")
		return Token::IntM;
	else if (_name == "uint8" || _name == "uint16" || _name == "uint32" || _name == "uint64" || _name == "uint128" || _name == "uint256")
		return Token::UIntM;	

	auto it = keywords.find(_name);
	return it == keywords.end() ? Token::Identifier : it->second;
}

char const* minisolc::tokenToString(Token tok) {
	switch (tok) {
#define T(name, string, precedence) \
	case Token::name:               \
		return string;
		TOKEN_LIST(T, T)
#undef T
	default: // Token::NUM_TOKENS:
		return "";
	}
}