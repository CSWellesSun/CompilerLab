#pragma once

#include "lexer/Token.h"
#include <memory>
#include <string>

namespace minisolc {

struct Location {
	std::shared_ptr<std::string> m_line;
	size_t m_line_idx;
	size_t m_start;
	size_t m_end;

	Location(std::shared_ptr<std::string> line, size_t lineIdx, size_t start, size_t end)
		: m_line(line), m_line_idx(lineIdx), m_start(start), m_end(end){};
};

struct TokenInfo {
	Token m_tok;
	std::string m_val;
	Location m_loc;

	TokenInfo(const Token tok, const std::string& v, std::shared_ptr<std::string> line, size_t lineIdx, size_t start, size_t end)
		: m_tok(tok), m_val(v), m_loc(line, lineIdx, start, end){};
	TokenInfo(const Token tok, std::string&& v, std::shared_ptr<std::string> line, size_t lineIdx, size_t start, size_t end)
		: m_tok(tok), m_val(v), m_loc(line, lineIdx, start, end){};
};

}