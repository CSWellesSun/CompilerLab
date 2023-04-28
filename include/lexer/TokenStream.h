#pragma once

#include "Token.h"
#include "common/common.h"
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
		m_line_start = m_source.cbegin();
		tokenize();
		mtokeniter = m_tokens.cbegin();
	}

	TokenInfo curTokInfo() const {
		if (mtokeniter == m_tokens.cend())
			return TokenInfo(Token::EOS, "", nullptr, 0, 0, 0);
		return *mtokeniter;
	}

	Token curTok() const {
		if (mtokeniter == m_tokens.cend())
			return Token::EOS;
		return mtokeniter->m_tok;
	}
	std::string curVal() const {
		if (mtokeniter == m_tokens.cend())
			return "";
		return mtokeniter->m_val;
	}
	// std::string curLine() const {
	// 	if (mtokeniter == m_tokens.cend() || m_curline == m_lines.cend())
	// 		return "";
	// 	return *m_curline;
	// }
	Token peekTok(size_t count) const {
		if (mtokeniter == m_tokens.cend() || mtokeniter + count >= m_tokens.cend())
			return Token::EOS;
		return (mtokeniter + count)->m_tok;
	}
	std::string peekVal(size_t count) const {
		if (mtokeniter == m_tokens.cend() || mtokeniter + count >= m_tokens.cend())
			return "";
		return (mtokeniter + count)->m_val;
	}

	bool advance() {
		if (mtokeniter < m_tokens.cend()) {
			++mtokeniter;
		}
		return mtokeniter != m_tokens.cend();
	}

	size_t pos() const { return mtokeniter - m_tokens.cbegin(); }
	void setPos(size_t pos) {
		if (pos < m_tokens.size())
			mtokeniter = pos + m_tokens.cbegin();
		else {
			mtokeniter = m_tokens.cend();
		}
	}
	bool eof() const { return mtokeniter == m_tokens.cend(); }
	bool error() const { return m_error; }
	void dump() const {
		for (const auto& tokenInfo: m_tokens) {
			std::cout << tokenInfo.m_val << " ";
		}
	}

private:
	void tokenize();
	void tokenizeKeywordIdent();
	bool tokenizeNumber();
	bool tokenizeString();
	void skipSpace();
	void readLines();
	bool skipAnnotation();
	void addToken(Token tok, std::string val) {
		m_tokens.push_back({
			tok,
			val,
			*m_curline,
			(size_t)(m_curline - m_lines.cbegin() + 1),
			(size_t)(m_striter - m_line_start),
			m_striter - m_line_start + val.length()});
	};

	std::string m_source;
	std::vector<std::shared_ptr<std::string>> m_lines;
	std::vector<std::shared_ptr<std::string>>::const_iterator m_curline;

	std::string::const_iterator m_line_start; // 当前行的起始位置（相对于m_source）
	std::string::const_iterator m_striter;

	std::vector<TokenInfo> m_tokens;
	std::vector<TokenInfo>::const_iterator mtokeniter;

	bool m_error = false;
};

}