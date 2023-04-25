#include "lexer/TokenStream.h"
#include "lexer/Token.h"

#include <algorithm>
#include <cctype>
#include <iostream>

using namespace minisolc;

void TokenStream::tokenize() {
	while (m_striter != m_source.end() && !m_error) {
		const char c = *m_striter;
		switch (c) {
		case '=':
			++m_striter; // advance
			m_tokens.push_back({Token::Assign, "="});
			break;
		case '(':
			++m_striter;
			m_tokens.push_back({Token::LParen, "("});
			break;
		case ')':
			++m_striter;
			m_tokens.push_back({Token::RParen, ")"});
			break;
		case '[':
			++m_striter;
			m_tokens.push_back({Token::LBrack, "["});
			break;
		case ']':
			++m_striter;
			m_tokens.push_back({Token::RBrack, "]"});
			break;
		case '{':
			++m_striter;
			m_tokens.push_back({Token::LBrace, "{"});
			break;
		case '}':
			++m_striter;
			m_tokens.push_back({Token::RBrace, "}"});
			break;
		case ';':
			++m_striter;
			m_tokens.push_back({Token::Semicolon, ";"});
			break;
		case ',':
			++m_striter;
			m_tokens.push_back({Token::Comma, ","});
			break;
		case '\0':
			++m_striter;
			break;
		case '\"':
			m_error = !tokenizeString();
			break;
		default: {
			if (isalus(c)) {
				tokenizeKeywordIdent();
			} else if (isdigit(c)) {
				m_error = !tokenizeNumber();
			} else if (isspace(c)) {
				skipSpace();
			} else {
				m_error = true;
			}
			break;
		}
		}
		// LOG_INFO("find token: %s", m_tokens.back().val.c_str());
	}
}

void TokenStream::tokenizeKeywordIdent() {
	auto right_bound = std::find_if_not(m_striter, m_source.cend(), [](const char ch) { return isalnumus(ch); });
	std::string val = std::string(m_striter, right_bound);
	m_tokens.emplace_back(keywordByName(val), val);
	m_striter = right_bound;
}

bool TokenStream::tokenizeNumber() {
	bool res = true;
	auto right_bound = std::find_if(m_striter, m_source.cend(), [](const char ch) { return issep(ch); });
	std::string val = std::string(m_striter, right_bound);
	if (val.size() > 1) {
		try {
			if (val[0] == '0') {
				if (val[1] == 'x' || val[1] == 'X') {
					// Hexadecimal
					std::stoll(val, 0, 16);
				} else {
					// Octal
					std::stoll(val, 0, 8);
				}
			} else {
				std::stoll(val);
			}
		} catch (...) {
			LOG_WARNING("Cannot tokenize the number.");
			res = false;
		}
	}
	m_tokens.emplace_back(Token::Number, std::move(val));
	m_striter = right_bound;
	return res;
}

void TokenStream::skipSpace() {
	m_striter = std::find_if_not(m_striter, m_source.cend(), [](const char ch) { return isspace(ch); });
}

bool TokenStream::tokenizeString() {
	auto right_quot = std::find(m_striter + 1, m_source.cend(), '\"');
	if (right_quot == m_source.end())
	{
		LOG_WARNING("Missing '\"'");
		return false;
	}
	std::string&& val = std::string(m_striter, right_quot + 1);
	m_tokens.emplace_back(Token::StringLiteral, std::move(val));
	m_striter = right_quot + 1;
	return true;
}