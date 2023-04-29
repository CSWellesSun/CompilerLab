#pragma once

#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "common.h"
#include "defs.h"
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
		std::cout << RED << "error: " << RESET << '\n';

		std::string head = std::to_string(m_tokinfo.m_loc.m_line_idx) + " | ";
		std::cout << head << *(m_tokinfo.m_loc.m_line) << '\n';

		head = std::string(head.size(), ' ');
		std::cout << head;
		for (size_t i = 0; i < m_tokinfo.m_loc.m_start; i++) {
			std::cout << ' ';
		}
		std::cout << RED;
		for (size_t i = m_tokinfo.m_loc.m_start; i < m_tokinfo.m_loc.m_end; i++) {
			std::cout << '^';
		}
		std::cout << RESET << '\n';

		std::cout << head;
		for (size_t i = 0; i < m_tokinfo.m_loc.m_start; i++) {
			std::cout << ' ';
		}
		std::cout << YELLOW << msg << RESET << '\n';
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
