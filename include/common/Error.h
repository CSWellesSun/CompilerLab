#pragma once

#include <corecrt.h>
#include <exception>
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
	void printErrorLine(std::string msg = "") const {
		std::string filename = m_tokinfo.m_loc.m_line->fileName;
		std::string line = m_tokinfo.m_loc.m_line->source;
		size_t idx = m_tokinfo.m_loc.m_line->lineNumber;
		size_t start = m_tokinfo.m_loc.m_start;
		size_t end = m_tokinfo.m_loc.m_end;
		std::shared_ptr<Line> include = m_tokinfo.m_loc.m_line->includeLine;

		// error:
		std::cout << "╭─ " << DARKRED << "ERROR" << RESET;
		std::cout << " In file " << filename << ", line " << idx << std::endl;

		// include:
		while (include != nullptr) {
			std::cout << "│        included from " << include->fileName << ", line " << include->lineNumber << std::endl;
			include = include->includeLine;
		}

		// line:
		std::string head = "╰─ " + std::to_string(idx) + " ";
		std::cout << head;
		std::cout << GRAY << line.substr(0, start) << RESET << DARKRED << line.substr(start, end - start) << RESET
				  << GRAY << line.substr(end) << RESET;

		// message:
		head = std::string(head.length() - 4, ' ');
		std::cout << head;
		for (size_t i = 0; i < start; i++) {
			std::cout << " ";
		}
		std::cout << "╰─ ";
		std::cout << DARKGREEN << msg << RESET << std::endl;
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
		ss << "Expect token:";
		for (auto tok: m_expectTok) {
			ss << ' ' << tokenToString(tok);
		}
		ss << ", but got: " << tokenToString(m_tokinfo.m_tok) << '\n';

		printErrorLine(ss.str());
	}

private:
	std::vector<Token> m_expectTok;
};

class ContractDefinitionParseError: public ParseError {
public:
	ContractDefinitionParseError(TokenInfo tokInfo): ParseError(tokInfo) {}
	void print() const override {
		std::stringstream ss;
		ss << "Expect function definition or variable declaration,";
		ss << " but got: " << tokenToString(m_tokinfo.m_tok) << '\n';

		printErrorLine(ss.str());
	}
};

class Warning {};
}
