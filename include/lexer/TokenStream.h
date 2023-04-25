#pragma once

#include "CharStream.h"
#include "Token.h"
#include <corecrt.h>
#include <iostream>
#include <vector>


namespace minisolc {

class TokenStream {
public:
	TokenStream(CharStream& source): m_source(source) {
		tokenize();
		m_len = m_tokens.size();
		if (m_error || m_len == 0) {
			m_eof = true;
		}
	}

	Token curTok() {
		if (m_eof)
			return Token::EOS;
		return m_tokens.at(m_pos).token;
	}
	std::string curVal() {
		if (m_eof)
			return "";
		return m_tokens.at(m_pos).val;
	}
	Token peekTok(size_t count) {
		if (m_eof || m_pos + count >= m_len)
			return Token::EOS;
		return m_tokens.at(m_pos + count).token;
	}
	std::string peekVal(size_t count) {
		if (m_eof || m_pos + count >= m_len)
			return "";
		return m_tokens.at(m_pos + count).val;
	}
	bool advance() {
		if (m_pos < m_len) {
			m_pos++;
			if (m_pos >= m_len)
				m_eof = true;
		}
		return !m_eof;
	}


	size_t pos() { return m_pos; }
	void setPos(size_t pos) {
		if (pos < m_len)
			m_pos = pos;
		else {
			m_pos = m_len;
			m_eof = true;
		}
	}
	bool eof() { return m_eof; }
	bool error() { return m_error; }
	void dump() {
		std::cout << m_len << std::endl;
		for (auto tokenInfo: m_tokens) {
			std::cout << tokenInfo.val << " ";
		}
	}

private:
	void tokenize();
	void tokenizeKeywordIdent();
	bool tokenizeNumber();
	bool tokenizeString();
	void skipSpace();

	struct TokenInfo {
		Token token;
		std::string val;
	};

	CharStream& m_source;
	std::vector<TokenInfo> m_tokens;
	bool m_error{false};
	size_t m_pos{0};
	size_t m_len{0};
	bool m_eof{false};
};

}