#pragma once

#include "Token.h"
#include "common/Defs.h"
#include "preprocess/Preprocess.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

namespace minisolc {

class TokenStream {
public:
	TokenStream(Preprocess& preprocess) {
		m_source = preprocess.source();
		m_striter = m_source.cbegin();
		tokenize();
		addToken(Token::EOS, "");
		mtokeniter = m_tokens.cbegin();
	}

	TokenInfo curTokInfo() const {
		if (mtokeniter == m_tokens.cend())
			return TokenInfo(Token::EOS, "", nullptr, 0, 0);
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
	void Dump() const {
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
	bool skipAnnotation();
	void addToken(Token tok, std::string val) {
		m_tokens.push_back({tok, val, m_striter.line(), m_striter.linePos(), m_striter.linePos() + val.length()});
	};

	/// 全局信息
	bool m_error = false; // 是否出错

	CharStream m_source;
	CharStream::const_iterator m_striter;

	std::vector<TokenInfo> m_tokens;
	std::vector<TokenInfo>::const_iterator mtokeniter;
};

}