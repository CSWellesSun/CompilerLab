#pragma once


#include "Token.h"
#include "common/defs.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>


namespace minisolc {

class TokenStream {
public:
	TokenStream(const std::string& filename) {
		std::ifstream file(filename);
		if (file.is_open()) {
			/* load input file stream into string */
			m_source = std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
			file.close();
		} else {
			LOG_WARNING("Program does not open file %s", filename.c_str());
			m_source = "";
			return;
		}

		readLines();
		m_curline = m_lines.cbegin();

		m_striter = m_source.cbegin();
		tokenize();
		mtokeniter = m_tokens.cbegin();
		eatEnter();
		if (!m_error)
			LOG_INFO("tokenize succeeds.");
	}

	Token curTok() const {
		if (mtokeniter == m_tokens.cend())
			return Token::EOS;
		return mtokeniter->token;
	}
	std::string curVal() const {
		if (mtokeniter == m_tokens.cend())
			return "";
		return mtokeniter->val;
	}
	std::string curLine() const {
		if (mtokeniter == m_tokens.cend() || m_curline == m_lines.cend())
			return "";
		return *m_curline;
	}
	// Token peekTok(size_t count) const {
	// 	if (mtokeniter == m_tokens.cend() || mtokeniter + count >= m_tokens.cend())
	// 		return Token::EOS;
	// 	return (mtokeniter + count)->token;
	// }
	// std::string peekVal(size_t count) const {
	// 	if (mtokeniter == m_tokens.cend() || mtokeniter + count >= m_tokens.cend())
	// 		return "";
	// 	return (mtokeniter + count)->val;
	// }

	bool advance() {
		if (mtokeniter < m_tokens.cend()) {
			++mtokeniter;
			eatEnter();
		}
		return mtokeniter != m_tokens.cend();
	}

	// size_t pos() const { return mtokeniter - m_tokens.cbegin(); }
	// void setPos(size_t pos) {
	// 	if (pos < m_tokens.size())
	// 		mtokeniter = pos + m_tokens.cbegin();
	// 	else {
	// 		mtokeniter = m_tokens.cend();
	// 	}
	// }
	bool eof() const { return mtokeniter == m_tokens.cend(); }
	bool error() const { return m_error; }
	void dump() const {
		for (const auto& tokenInfo: m_tokens) {
			if (tokenInfo.token != Token::Whitespace)
				std::cout << tokenInfo.val << " ";
		}
	}

private:
	void tokenize();
	void tokenizeKeywordIdent();
	bool tokenizeNumber();
	bool tokenizeString();
	void skipSpace();
	void readLines();
	void eatEnter() {
		while (mtokeniter < m_tokens.cend() && mtokeniter->token == Token::Whitespace) {
			++mtokeniter;
			++m_curline;
		}
	}

	struct TokenInfo {
		Token token;
		std::string val;

		TokenInfo(const Token tok, const std::string& v): token(tok), val(v){};
		TokenInfo(const Token tok, std::string&& v): token(tok), val(v){};
	};

	std::string m_source;
	std::vector<std::string> m_lines;
	std::vector<std::string>::const_iterator m_curline;

	std::string::const_iterator m_striter;

	std::vector<TokenInfo> m_tokens;
	std::vector<TokenInfo>::const_iterator mtokeniter;

	bool m_error = false;
};

}