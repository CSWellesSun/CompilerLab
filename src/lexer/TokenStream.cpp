#include "lexer/TokenStream.h"
#include "lexer/Token.h"

#include <cctype>
#include <iostream>

using namespace minisolc;

void TokenStream::tokenize() {
	while (!m_source.eof() && !m_error) {
		m_source.mark();
		char c = m_source.current();
		bool res;
		switch (c) {
		case '=':
			m_source.advance();
			m_tokens.push_back({Token::Assign, "="});
			break;
		case '(':
			m_source.advance();
			m_tokens.push_back({Token::LParen, "("});
			break;
		case ')':
			m_source.advance();
			m_tokens.push_back({Token::RParen, ")"});
			break;
		case '[':
			m_source.advance();
			m_tokens.push_back({Token::LBrack, "["});
			break;
		case ']':
			m_source.advance();
			m_tokens.push_back({Token::RBrack, "]"});
			break;
		case '{':
			m_source.advance();
			m_tokens.push_back({Token::LBrace, "{"});
			break;
		case '}':
			m_source.advance();
			m_tokens.push_back({Token::RBrace, "}"});
			break;
		case ';':
			m_source.advance();
			m_tokens.push_back({Token::Semicolon, ";"});
			break;
		case ',':
			m_source.advance();
			m_tokens.push_back({Token::Comma, ","});
			break;
		case '\0':
			m_source.advance();
			break;
		case '\"':
			res = tokenizeString();
			m_error = !res;
			break;
		default: {
			if (isalus(c)) {
				tokenizeKeywordIdent();
			} else if (isdigit(c)) {
				res = tokenizeNumber();
				m_error = !res;
			} else if (isspace(c)) { // 空格略过
				skipSpace();
			} else {
				m_error = true;
			}
			break;
		}
		}
	}
}

void TokenStream::tokenizeKeywordIdent() {
	while (!m_source.eof() && isalnumus(m_source.current())) {
		m_source.advance();
	}
	std::string val = m_source.text(m_source.markPos(), m_source.position());
	m_tokens.push_back({keywordByName(val), val});
}

bool TokenStream::tokenizeNumber() {
	bool res = true;

	if (m_source.current() == '0') { // 八进制数
		m_source.advance();
		if (m_source.eof() || issep(m_source.current())) { // zero
			m_tokens.push_back({Token::Number, "0"});
		} else if (m_source.current() == 'x' || m_source.current() == 'X') { // 十六进制数
			m_source.advance();
			if (m_source.eof() || !isxdigit(m_source.current())) { // 非法十六进制
				res = false;
			} else { // 读取十六进制数
				while (!m_source.eof() && isxdigit(m_source.current())) {
					m_source.advance();
				}
				if (m_source.eof() || issep(m_source.current())) { // 十六进制数结束后为eof或空或分号
					std::string val = m_source.text(m_source.markPos(), m_source.position());
					m_tokens.push_back({Token::Number, val});
				} else { // 非法十六进制
					res = false;
				}
			}
		} else if (isoct(m_source.current())) { // 八进制数
			while (!m_source.eof() && isoct(m_source.current())) {
				m_source.advance();
			}
			if (m_source.eof() || issep(m_source.current())) { // 八进制数结束后为eof或空或分号
				std::string val = m_source.text(m_source.markPos(), m_source.position());
				m_tokens.push_back({Token::Number, val});
			} else { // 非法八进制
				res = false;
			}
		} else { // error!
			res = false;
		}
	} else { // 十进制数
		while (!m_source.eof() && isdigit(m_source.current())) {
			m_source.advance();
		}
		if (m_source.eof() || issep(m_source.current())) { // 十进制数结束后为eof或空或分号
			std::string val = m_source.text(m_source.markPos(), m_source.position());
			m_tokens.push_back({Token::Number, val});
		} else { // 非法非进制
			res = false;
		}
	}

	return res;
}

void TokenStream::skipSpace() {
	while (!m_source.eof() && isspace(m_source.current())) {
		m_source.advance();
	}
}

bool TokenStream::tokenizeString() {
	while (!m_source.eof() && m_source.current() != '\"') {
		if (m_source.current() == '\r' || m_source.current() == '\n') {
			return false;
		}
		m_source.advance();
	}
	if (m_source.eof()) {
		return false;
	} else if (m_source.current() == '\"') {
		m_source.advance();
		std::string val = m_source.text(m_source.markPos(), m_source.position());
		m_tokens.push_back({Token::StringLiteral, val});
	} 	
	return true;
}