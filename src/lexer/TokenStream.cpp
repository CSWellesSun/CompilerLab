#include "lexer/TokenStream.h"
#include "common/Defs.h"
#include "lexer/Token.h"


#include <algorithm>
#include <cctype>
#include <ctype.h>
#include <iostream>
#include <memory>
#include <sstream>

using namespace minisolc;

void TokenStream::tokenize() {
	while (m_striter != m_source.cend() && !m_error) {
		switch (*m_striter) {
		case '=':
			++m_striter; // advance
			if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::Equal, "==");
				++m_striter;
			} else if (m_striter != m_source.cend() && *m_striter == '>') {
				addToken(Token::DoubleArrow, "=>");
				++m_striter;
			} else {
				addToken(Token::Assign, "=");
			}
			break;
		case '+':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::AssignAdd, "+=");
				++m_striter;
			} else if (m_striter != m_source.cend() && *m_striter == '+') {
				addToken(Token::Inc, "++");
				++m_striter;
			} else {
				addToken(Token::Add, "+");
			}
			break;
		case '-':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::AssignSub, "-=");
				++m_striter;
			} else if (m_striter != m_source.cend() && *m_striter == '>') {
				addToken(Token::RightArrow, "->");
				++m_striter;
			} else if (m_striter != m_source.cend() && *m_striter == '-') {
				addToken(Token::Dec, "--");
				++m_striter;
			} else {
				addToken(Token::Sub, "-");
			}
			break;
		case '*':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::AssignMul, "*=");
				++m_striter;
			} else if (m_striter != m_source.cend() && *m_striter == '*') {
				addToken(Token::Exp, "**");
				++m_striter;
			} else {
				addToken(Token::Mul, "*");
			}
			break;
		case '/':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::AssignDiv, "/=");
				++m_striter;
			} else if (*m_striter == '*' || *m_striter == '/') {
				m_error = !skipAnnotation();
			} else {
				addToken(Token::Div, "/");
			}
			break;
		case '%':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::AssignMod, "%=");
				++m_striter;
			} else {
				addToken(Token::Mod, "%");
			}
			break;
		case '!':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::NotEqual, "!=");
				++m_striter;
			} else {
				addToken(Token::Not, "!");
			}
			break;
		case '>':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '>') {
				if (m_striter + 1 != m_source.cend() && *(m_striter + 1) == '>') {
					if (m_striter + 2 != m_source.cend() && *(m_striter + 2) == '=') {
						addToken(Token::AssignSar, ">>>=");
						m_striter += 3;
					} else {
						addToken(Token::SHR, ">>>");
						m_striter += 2;
					}
				} else if (m_striter + 1 != m_source.cend() && *(m_striter + 1) == '=') {
					addToken(Token::AssignShr, ">>=");
					m_striter += 2;
				} else {
					addToken(Token::SAR, ">>");
					++m_striter;
				}
			} else if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::GreaterThanOrEqual, ">=");
				++m_striter;
			} else {
				addToken(Token::GreaterThan, ">");
			}
			break;
		case '<':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '<') {
				if (m_striter + 1 != m_source.cend() && *(m_striter + 1) == '=') {
					addToken(Token::AssignShl, "<<=");
					m_striter += 2;
				} else {
					addToken(Token::SHL, "<<");
					++m_striter;
				}
			} else if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::LessThanOrEqual, "<=");
				++m_striter;
			} else {
				addToken(Token::LessThan, "<");
			}
			break;
		case '&':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '&') {
				addToken(Token::And, "&&");
				++m_striter;
			} else if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::AssignBitAnd, "&=");
				++m_striter;
			} else {
				addToken(Token::BitAnd, "&");
			}
			break;
		case '|':
			++m_striter;
			if (m_striter != m_source.cend() && *m_striter == '|') {
				addToken(Token::Or, "||");
				++m_striter;
			} else if (m_striter != m_source.cend() && *m_striter == '=') {
				addToken(Token::AssignBitOr, "|=");
				++m_striter;
			} else {
				addToken(Token::BitOr, "|");
			}
			break;
		case '~':
			++m_striter;
			addToken(Token::BitNot, "~");
			break;
		case '?':
			++m_striter;
			addToken(Token::Conditional, "?");
			break;
		case ':':
			++m_striter;
			addToken(Token::Colon, ":");
			break;
		case '(':
			++m_striter;
			addToken(Token::LParen, "(");
			break;
		case ')':
			++m_striter;
			addToken(Token::RParen, ")");
			break;
		case '[':
			++m_striter;
			addToken(Token::LBrack, "[");
			break;
		case ']':
			++m_striter;
			addToken(Token::RBrack, "]");
			break;
		case '{':
			++m_striter;
			addToken(Token::LBrace, "{");
			break;
		case '}':
			++m_striter;
			addToken(Token::RBrace, "}");
			break;
		case ';':
			++m_striter;
			addToken(Token::Semicolon, ";");
			break;
		case ',':
			++m_striter;
			addToken(Token::Comma, ",");
			break;
		case '\0':
			++m_striter;
			break;
		case '\"':
			m_error = !tokenizeString();
			break;
		case '.':
			++m_striter;
			addToken(Token::Period, ".");
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
	if (!m_error)
		LOG_INFO("Tokenize Succeeds.");
}

void TokenStream::tokenizeKeywordIdent() {
	auto right_bound = std::find_if_not(m_striter, m_source.cend(), [](const char ch) { return isalnumus(ch); });
	std::string val = std::string(m_striter, right_bound);
	addToken(keywordByName(val), val);
	m_striter = right_bound;
}

bool TokenStream::tokenizeNumber() {
	bool res = true;
	bool floatFlag = false;

	auto right_bound = std::find_if(m_striter, m_source.cend(), [&](const char ch) {
		if (ch == '.') {
			floatFlag = true;
		}
		return issep(ch);
	});
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
			} else if (!floatFlag) {
				// Decimal
				std::stoll(val);
			} else {
				// Float
				std::stod(val);
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
	if (floatFlag)
		addToken(Token::DoubleNumber, std::move(val));
	else
		addToken(Token::IntNumber, std::move(val));
	m_striter = right_bound;
	return res;
}

void TokenStream::skipSpace() {
	while (m_striter != m_source.cend() && isspace(*m_striter)) {
		++m_striter;
	}
}

bool TokenStream::skipAnnotation() {
	CharStream::const_iterator right_pos;
	--m_striter;
	if (*m_striter == '/' && *(m_striter + 1) == '/') {
		/* single-line annotations */
		right_pos = m_source.find('\n', m_striter + 2);
	} else if (*m_striter == '/' && *(m_striter + 1) == '*') {
		/* multi-line annotations */
		right_pos = m_source.find("*/", m_striter + 2);
		right_pos += 2;
	} else {
		LOG_WARNING("Parse Annotation Fails.");
		return false;
	}
	if (right_pos != m_source.cend()) {
		m_striter = right_pos;
		return true;
	} else {
		LOG_WARNING("Parse Annotation Fails.");
		return false;
	}
}

bool TokenStream::tokenizeString() {
	auto right_quot = std::find(m_striter + 1, m_source.cend(), '\"');
	if (right_quot == m_source.cend()) {
		LOG_WARNING("Missing '\"'");
		return false;
	}
	std::string&& val = std::string(m_striter, right_quot + 1);
	addToken(Token::StringLiteral, std::move(val));
	m_striter = right_quot + 1;
	return true;
}