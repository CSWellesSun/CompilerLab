#include "lexer/TokenStream.h"
#include "common/defs.h"
#include "lexer/Token.h"


#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

using namespace minisolc;

void TokenStream::tokenize() {
	while (m_striter != m_source.end() && !m_error) {
		switch (*m_striter) {
		case '=':
			++m_striter; // advance
			if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::Equal, "=="});
				++m_striter;
			} else if (m_striter != m_source.end() && *m_striter == '>') {
				m_tokens.push_back({Token::DoubleArrow, "=>"});
				++m_striter;
			} else {
				m_tokens.push_back({Token::Assign, "="});
			}
			break;
		case '+':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::AssignAdd, "+="});
				++m_striter;
			} else if (m_striter != m_source.end() && *m_striter == '+') {
				m_tokens.push_back({Token::Inc, "++"});
				++m_striter;
			} else {
				m_tokens.push_back({Token::Add, "+"});
			}
			break;
		case '-':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::AssignSub, "-="});
				++m_striter;
			} else if (m_striter != m_source.end() && *m_striter == '>') {
				m_tokens.push_back({Token::RightArrow, "->"});
				++m_striter;
			} else if (m_striter != m_source.end() && *m_striter == '-') {
				m_tokens.push_back({Token::Dec, "--"});
				++m_striter;
			} else {
				m_tokens.push_back({Token::Sub, "-"});
			}
			break;
		case '*':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::AssignMul, "*="});
				++m_striter;
			} else if (m_striter != m_source.end() && *m_striter == '*') {
				m_tokens.push_back({Token::Exp, "**"});
				++m_striter;
			} else {
				m_tokens.push_back({Token::Mul, "*"});
			}
			break;
		case '/':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::AssignDiv, "/="});
				++m_striter;
			} else if (*m_striter == '*' || *m_striter == '/') {
				m_error = !skipAnnotation();
			} else {
				m_tokens.push_back({Token::Div, "/"});
			}
			break;
		case '%':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::AssignMod, "%="});
				++m_striter;
			} else {
				m_tokens.push_back({Token::Mod, "%"});
			}
			break;
		case '!':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::NotEqual, "!="});
				++m_striter;
			} else {
				m_tokens.push_back({Token::Not, "!"});
			}
			break;
		case '>':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::GreaterThanOrEqual, ">="});
				++m_striter;
			} else if (m_striter != m_source.end() && *m_striter == '>') {
				if (m_striter + 1 != m_source.end() && *(m_striter + 1) == '=') {
					if (m_striter + 2 != m_source.end() && *(m_striter + 2) == '>') {
						m_tokens.push_back({Token::AssignSar, ">>>="});
						m_striter += 3;
					} else {
						m_tokens.push_back({Token::AssignShr, ">>="});
						m_striter += 2;
					}
				} else {
					m_tokens.push_back({Token::SAR, ">>"});
					++m_striter;
				}
			} else {
				m_tokens.push_back({Token::GreaterThan, ">"});
			}
			break;
		case '<':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::LessThanOrEqual, "<="});
				++m_striter;
			} else if (m_striter != m_source.end() && *m_striter == '<') {
				if (m_striter + 1 != m_source.end() && *(m_striter + 1) == '=') {
					m_tokens.push_back({Token::AssignShl, "<<="});
					m_striter += 2;
				} else {
					m_tokens.push_back({Token::SHL, "<<"});
					++m_striter;
				}
			} else {
				m_tokens.push_back({Token::LessThan, "<"});
			}
			break;
		case '&':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '&') {
				m_tokens.push_back({Token::And, "&&"});
				++m_striter;
			} else if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::AssignBitAnd, "&="});
				++m_striter;
			} else {
				m_tokens.push_back({Token::BitAnd, "&"});
			}
			break;
		case '|':
			++m_striter;
			if (m_striter != m_source.end() && *m_striter == '|') {
				m_tokens.push_back({Token::Or, "||"});
				++m_striter;
			} else if (m_striter != m_source.end() && *m_striter == '=') {
				m_tokens.push_back({Token::AssignBitOr, "|="});
				++m_striter;
			} else {
				m_tokens.push_back({Token::BitOr, "|"});
			}
			break;
		case '~':
			++m_striter;
			m_tokens.push_back({Token::BitNot, "~"});
			break;
		case '?':
			++m_striter;
			m_tokens.push_back({Token::Conditional, "?"});
			break;
		case ':':
			++m_striter;
			m_tokens.push_back({Token::Colon, ":"});
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
		case '\n':
			++m_striter;
			m_tokens.push_back({Token::Whitespace, "\n"});
			break;
		default: {
			if (isalus(*m_striter)) {
				/* keyword or identifier */
				tokenizeKeywordIdent();
			} else if (isdigit(*m_striter)) {
				/* number */
				m_error = !tokenizeNumber();
			} else if (isspace(*m_striter)) {
				/* spaces */
				skipSpace();
			} else {
				LOG_WARNING("Invalid Character.");
				m_error = true;
			}
			break;
		}
		}
		// LOG_INFO("find token: %s", m_tokens.back().val.c_str());
	}
	m_tokens.push_back({Token::EOS, ""});
	if (!m_error)
		LOG_INFO("Tokenize Succeeds.");
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
				// Decimal
				std::stoll(val);
			}
			/* std::stoll will throw except std::invalid_argument 
			   if no conversion could be performed; and 
			   throw std::out_of_range if the converted value would fall 
			   out of the range of the result type. */
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

bool TokenStream::skipAnnotation() {
	size_t right_pos;
	--m_striter;
	if (*m_striter == '/' && *(m_striter + 1) == '/') {
		/* single-line annotations */
		right_pos = m_source.find('\n', m_striter + 2 - m_source.begin());
	} else if (*m_striter == '/' && *(m_striter + 1) == '*') {
		/* multi-line annotations */
		right_pos = m_source.find("*/", m_striter + 2 - m_source.begin());
	} else {
		LOG_WARNING("Parse Annotation Fails.");
		return false;
	}
	if (right_pos != std::string::npos) {
		m_striter = m_source.begin() + right_pos + 2;
		// LOG_INFO("Skip Annotation.");
		return true;
	} else {
		LOG_WARNING("Parse Annotation Fails.");
		return false;
	}
}

bool TokenStream::tokenizeString() {
	auto right_quot = std::find(m_striter + 1, m_source.cend(), '\"');
	if (right_quot == m_source.end()) {
		LOG_WARNING("Missing '\"'");
		return false;
	}
	std::string&& val = std::string(m_striter, right_quot + 1);
	m_tokens.emplace_back(Token::StringLiteral, std::move(val));
	m_striter = right_quot + 1;
	return true;
}

void TokenStream::readLines() {
	std::string line;
	std::stringstream ss(m_source);
	while (std::getline(ss, line)) {
		m_lines.push_back(std::move(line));
	}
}