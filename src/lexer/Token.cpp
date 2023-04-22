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