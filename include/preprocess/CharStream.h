#pragma once

#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace minisolc {

struct Line {
	std::string source;
	size_t lineNumber;
	std::string fileName;
	std::shared_ptr<Line> includeLine;

	Line(std::string source, size_t lineNumber, std::string fileName, std::shared_ptr<Line> includeLine = nullptr)
		: source(source), lineNumber(lineNumber), fileName(fileName), includeLine(includeLine) {}
};

class CharStream {
public:
	CharStream() {}
	void push_back(std::shared_ptr<Line> line) { m_lines.emplace_back(line); }

	std::string str() const {
		std::string result;
		for (const auto& line: m_lines) {
			result += line->source;
		}
		return result;
	}

	void Dump() const { std::cout << str() << std::endl; }

	class const_iterator {
	public:
		using value_type = char;
		using difference_type = std::ptrdiff_t;
		using pointer = const char*;
		using reference = const char&;
		using iterator_category = std::input_iterator_tag;

		const_iterator(): stream(nullptr), lineIndex(0), charIndex(0) {}
		const_iterator(const CharStream* stream, std::size_t lineIndex, std::size_t charIndex)
			: stream(stream), lineIndex(lineIndex), charIndex(charIndex) {}

		char operator*() const { return stream->m_lines[lineIndex]->source[charIndex]; }

		const_iterator operator+(int n) const {
			const_iterator result = *this;
			for (int i = 0; i < n; ++i) {
				++result;
			}
			return result;
		}

		const_iterator& operator++() {
			if (lineIndex >= stream->m_lines.size()) {
				return *this;
			}
			if (charIndex < stream->m_lines[lineIndex]->source.size() - 1) {
				++charIndex;
			} else if (lineIndex < stream->m_lines.size() - 1) {
				++lineIndex;
				charIndex = 0;
			} else {
				stream = nullptr;
				lineIndex = 0;
				charIndex = 0;
			}
			return *this;
		}

		const_iterator operator++(int) {
			const_iterator old = *this;
			++(*this);
			return old;
		}

		const_iterator& operator+=(int n) {
			for (int i = 0; i < n; ++i) {
				++(*this);
			}
			return *this;
		}

		const_iterator& operator--() {
			if (lineIndex == 0 && charIndex == 0) {
				return *this;
			}
			if (charIndex > 0) {
				--charIndex;
			} else {
				--lineIndex;
				charIndex = stream->m_lines[lineIndex]->source.size() - 1;
			}
			return *this;
		}

		const_iterator operator--(int) {
			const_iterator old = *this;
			--(*this);
			return old;
		}

		const_iterator& operator-=(int n) {
			for (int i = 0; i < n; ++i) {
				--(*this);
			}
			return *this;
		}

		const_iterator operator-(int n) const {
			const_iterator result = *this;
			for (int i = 0; i < n; ++i) {
				--result;
			}
			return result;
		}

		bool operator==(const const_iterator& other) const {
			return stream == other.stream && lineIndex == other.lineIndex && charIndex == other.charIndex;
		}

		bool operator!=(const const_iterator& other) const { return !(*this == other); }

		const_iterator find(char c) const {
			const_iterator it = *this;
			while (it != stream->cend()) {
				if (*it == c) {
					return it;
				}
				++it;
			}
			return stream->cend();
		}

		const_iterator find(std::string s) const {
			const_iterator it = *this;
			while (it != stream->cend()) {
				bool found = true;
				for (size_t i = 0; i < s.size(); ++i) {
					if (it + i == stream->cend() || *(it + i) != s[i]) {
						found = false;
						break;
					}
				}
				if (found) {
					return it;
				}
				++it;
			}
			return stream->cend();
		}

		size_t linePos() const { return charIndex; }
		std::shared_ptr<Line> line() const { 
			if (stream == nullptr) return nullptr;
			return stream->m_lines[lineIndex]; 
		}

		const CharStream* stream;
		std::size_t lineIndex;
		std::size_t charIndex;
	private:
	};

	const_iterator cbegin() const { return const_iterator(this, 0, 0); }

	const_iterator cend() const { return const_iterator(nullptr, 0, 0); }

	const_iterator find(char c, const_iterator it) const { return it.find(c); }

	const_iterator find(std::string s, const_iterator it) const { return it.find(s); }

private:
	std::vector<std::shared_ptr<Line>> m_lines;
};


}