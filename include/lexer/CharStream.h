#pragma once

#include <corecrt.h>
#include <fstream>
#include <string>

namespace minisolc {

class CharStream {
private:
	std::string m_source;
	size_t m_len{0};
	size_t m_pos{0};
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

		m_len = m_source.length();
		if (m_len == 0) {
			m_eof = true;
		}
	};

	size_t position() { return m_pos; }
	bool eof() { return m_eof; }
	char current() { return m_source[m_pos]; }
	void mark() { m_mark = m_pos; }
	size_t markPos() { return m_mark; }
	bool advance() {
		if (m_pos < m_len) {
			m_pos++;
		} else {
			m_eof = true;
		}
		return !m_eof;
	}
	bool rollback(size_t count) {
		bool res = true;
		if (count > m_pos) {
			res = false;
		} else {
			m_pos -= count;
		}
		return res;
	}

	std::string text(size_t start, size_t end) {
		std::string res = "";
		if (start < end && start < m_len) {
			if (end > m_len) {
				end = m_len;
			}
			res = m_source.substr(start, end - start);
		}
		return res;
	}

	std::string toString() { return m_source; }
};

}