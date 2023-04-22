#pragma once

#include <corecrt.h>
#include <fstream>
#include <string>

namespace minisolc {

class CharStream {
private:
	std::string m_source;
	size_t m_length{0};
	size_t m_position{0};
	size_t m_mark{0};
	bool m_eof{false};


public:
	enum SourceKind { Memory, File };

	CharStream(std::string source, SourceKind kind) {
		if (kind == File) { // 使用文件打开
			std::ifstream file(source);
			if (file.is_open()) {
				std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
				file.close(); // 关闭文件
				m_source = std::move(content);
			} else {
				m_eof = true;
				return;
			}
		} else {
			m_source = std::move(source);
		}

		m_length = m_source.length();
		if (m_length == 0) {
			m_eof = true;
		}
	};

	size_t position() { return m_position; }
	bool eof() { return m_eof; }
	char current() { return m_source[m_position]; }
	void mark() { m_mark = m_position; }
	size_t markPos() { return m_mark; }
	bool advance() {
		if (m_position < m_length) {
			m_position++;
		} else {
			m_eof = true;
		}
		return !m_eof;
	}
	bool rollback(size_t count) {
		bool res = true;
		if (count > m_position) {
			res = false;
		} else {
			m_position -= count;
		}
		return res;
	}

	std::string text(size_t start, size_t end) {
		std::string result = "";
		if (start < end && start < m_length) {
			if (end > m_length) {
				end = m_length;
			}
			result = m_source.substr(start, end - start);
		}
		return result;
	}

	std::string toString() { return m_source; }
};

}