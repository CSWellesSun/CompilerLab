#pragma once

#include "Scanner.h"

#include "CharStream.h"
#include "Token.h"

namespace minisolc {

class Scanner {
public:
	Scanner(CharStream& source): m_source(source) {
        next();
        // next();
        // next();
    }
    void next();
    Token current() { return m_token[Current].token; };
    // Token peekNext() { return m_token[Next].token; };
    // Token peekNextNext() { return m_token[NextNext].token; };

private:
	struct TokenDesc {
		Token token;
		std::string literal;
	};

	enum TokenIndex { Current, Next, NextNext };
	TokenDesc m_token[3];

	CharStream& m_source;

    bool m_eof{false};
};

} // namespace minisolc