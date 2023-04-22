#pragma once

#include "CharStream.h"
#include "Token.h"
#include <iostream>
#include <vector>


namespace minisolc {

class TokenStream {
public:
	TokenStream(CharStream& source): m_source(source) { tokenize(); }

	void dump() {
		std::cout << m_tokens.size() << std::endl;
		for (auto tokenInfo: m_tokens) {
			std::cout << tokenInfo.val << " ";
		}
	}

private:
	void tokenize();
	void tokenizeKeywordIdent();
	bool tokenizeNumber();
	void skipSpace();

	struct TokenInfo {
		Token token;
		std::string val;
	};

	CharStream& m_source;
	std::vector<TokenInfo> m_tokens;
	bool m_error{false};
};

}