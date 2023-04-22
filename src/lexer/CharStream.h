#pragma once

#include <string>

namespace minisolc {

class CharStream {
private:
	std::string m_source;
	size_t m_length{0};
	size_t m_position{0};
	bool m_eof{false};

public:
	CharStream(std::string source) {
		m_source = std::move(source);
		m_length = m_source.length();
		if (m_length == 0) {
			m_eof = true;
		}
	};

	size_t position() { return m_position; }
	bool isEof() { return m_eof; }
	char getCurrentChar() { return m_source[m_position]; }
	bool advance() {
		if (m_position < m_length) {
			m_position++;
		} else {
			m_eof = true;
		}
		return m_eof;
	}
};

}