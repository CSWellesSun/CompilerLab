#pragma once

#include <corecrt.h>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>


#include "Defs.h"
#include "lexer/Token.h"


namespace minisolc {

class Error: public std::exception {
public:
	Error(const std::string& msg): m_msg(msg) {}
	virtual const char* what() const noexcept { return m_msg.c_str(); }
	virtual void print() const = 0;

private:
	std::string m_msg;
};

class ParseError: public Error {
public:
	ParseError(TokenInfo tokInfo): Error("Parse Error"), m_tokinfo(tokInfo) {}
	void print() const {};

protected:
	void printErrorLine(std::string msg = "", std::string shortMsg = "") const {
		std::string filename = m_tokinfo.m_loc.m_line->fileName;
		std::string line = m_tokinfo.m_loc.m_line->source;
		size_t idx = m_tokinfo.m_loc.m_line->lineNumber;
		size_t start = m_tokinfo.m_loc.m_start;
		size_t end = m_tokinfo.m_loc.m_end;
		std::shared_ptr<Line> include = m_tokinfo.m_loc.m_line->includeLine;

		// error:
		std::cout << DARKRED << "ERROR: " << RESET << msg;
		std::string head = std::to_string(idx);
		std::cout << DARKGRAY << std::string(head.length() + 1, ' ') << "╭─ " << RESET << GRAY << "[" << RESET
				  << filename << ":" << idx << ":" << start << GRAY << "]" << RESET;

		// include:
		while (include != nullptr) {
			std::cout << GRAY << " -> " << RESET << GRAY << "[" << RESET << include->fileName << ":"
					  << include->lineNumber << GRAY << "]" << RESET;
			include = include->includeLine;
		}
		std::cout << std::endl;

		// line:
		std::cout << DARKGRAY << head << " │ " << RESET;
		std::cout << line.substr(0, start) << DARKRED << line.substr(start, end - start) << RESET << line.substr(end);

		// message:
		std::cout << DARKGRAY << std::string(head.length(), ' ') << " · ";
		for (size_t i = 0; i < start; i++) {
			std::cout << " ";
		}
		std::cout << RESET << RED << "╰─ " << RESET;
		std::cout << shortMsg;

		// end:
		std::cout << DARKGRAY << std::string(head.length(), ' ') << "─╯ " << RESET << std::endl;
	}

	TokenInfo m_tokinfo;
};

class UnexpectedToken: public ParseError {
public:
	/// TODO: 有可能有Token和func同时的情况以及前者为vector的情况
	UnexpectedToken(TokenInfo tokInfo, Token expectTok): ParseError(tokInfo) { m_expectTok.push_back(expectTok); }
	UnexpectedToken(TokenInfo tokInfo, std::function<bool(Token)> func): ParseError(tokInfo) {
		for (int i = 0; i < static_cast<int>(Token::NUM_TOKENS); i++) {
			if (func(static_cast<Token>(i))) {
				m_expectTok.push_back(static_cast<Token>(i));
			}
		}
	}

	void print() const override {
		std::stringstream ss;
		std::string shortMsg;

		ss << "Unexpected " << RED << tokenToString(m_tokinfo.m_tok) << RESET;
		shortMsg = ss.str() + "\n";
		ss << " while parsing pattern, expected ";
		if (m_expectTok.size() == 1) {
			ss << CYAN << tokenToString(m_expectTok[0]) << RESET;
		} else {
			ss << " one of ";
			for (auto tok: m_expectTok) {
				ss << CYAN << tokenToString(tok) << RESET;
				if (tok != m_expectTok.back()) {
					ss << ", ";
				}
			}
		}
		ss << "\n";

		printErrorLine(ss.str(), shortMsg);
	}

private:
	std::vector<Token> m_expectTok;
};

class ContractDefinitionParseError: public ParseError {
public:
	ContractDefinitionParseError(TokenInfo tokInfo): ParseError(tokInfo) {}
	void print() const override {
		std::stringstream ss;
		std::string shortMsg;

		ss << "Unexpected " << RED << tokenToString(m_tokinfo.m_tok) << RESET;
		shortMsg = ss.str() + "\n";
		ss << " while parsing pattern, expected " << CYAN << "function definition " << RESET << "or " << CYAN
		   << "variable declaration" << RESET << "\n";

		printErrorLine(ss.str(), shortMsg);
	}
};

class Warning {};
}
