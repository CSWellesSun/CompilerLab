#pragma once

#include "CharStream.h"
#include "Token.h"

namespace minisolc {

class Lexer {
public:
	Lexer(CharStream& source): m_source(source) {}

private:
	struct TokenInfo {
		Token token;
		std::string val;
	};

	CharStream& m_source;

	bool m_eof{false};
};

} // namespace minisolc