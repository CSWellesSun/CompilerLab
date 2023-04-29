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
		std::cout << RED << "error: " << RESET << std::endl;

		std::string head = std::to_string(m_tokinfo.m_loc.m_line_idx) + " | ";
		std::cout << head << *(m_tokinfo.m_loc.m_line) << std::endl;

		head = std::string(head.size(), ' ');
		std::cout << head;
		for (size_t i = 0; i < m_tokinfo.m_loc.m_start; i++) {
			std::cout << " ";
		}
		std::cout << RED;
		for (size_t i = m_tokinfo.m_loc.m_start; i < m_tokinfo.m_loc.m_end; i++) {
			std::cout << "^";
		}
		std::cout << RESET << std::endl;

		std::cout << head;
		for (size_t i = 0; i < m_tokinfo.m_loc.m_start; i++) {
			std::cout << " ";
		}
		std::cout << GREEN << msg << RESET << std::endl;
	}

	TokenInfo m_tokinfo;
};

class UnexpectedToken: public ParseError {
<<<<<<< HEAD
public:
	UnexpectedToken(const std::string& line, Token tok, Token expectTok)
		: ParseError(line, tok) {
        m_expectTok.push_back(expectTok);
    }
	UnexpectedToken(const std::string& line, Token tok, bool (*func)(Token))
		: ParseError(line, tok) {
        for (int i = 0; i < static_cast<int>(Token::NUM_TOKENS); ++i) {
            if (func(static_cast<Token>(i))) {
                m_expectTok.push_back(static_cast<Token>(i));
            }
        }
    }

	void print() const override {
		std::cout << m_line << '\n'
				  << "Expect token: ";
        for (auto tok : m_expectTok) {
            std::cout << tokenToString(tok) << ' ';
        }
		std::cout << '\n' 
		 		  << "But got: " << tokenToString(m_tok) << '\n';
=======
public: 
	/// TODO: 有可能有Token和func同时的情况以及前者为vector的情况
	UnexpectedToken(TokenInfo tokInfo, Token expectTok): ParseError(tokInfo) { m_expectTok.push_back(expectTok); }
	UnexpectedToken(TokenInfo tokInfo, bool (*func)(Token)): ParseError(tokInfo) {
		for (int i = 0; i < (int) Token::NUM_TOKENS; i++) {
			if (func((Token) i)) {
				m_expectTok.push_back((Token) i);
			}
		}
	}

	void print() const override {
		std::stringstream ss;
		ss << "Expect token:";
		for (auto tok: m_expectTok) {
			ss << " " << tokenToString(tok);
		}
		ss << ", but got: " << tokenToString(m_tokinfo.m_tok) << std::endl;

		printErrorLine(ss.str());
>>>>>>> c45277eeed906a3d0a3c8940b1cb8c900a0cf0be
	}

private:
	std::vector<Token> m_expectTok;
};

class ContractDefinitionParseError: public ParseError {
public:
	ContractDefinitionParseError(TokenInfo tokInfo): ParseError(tokInfo) {}
	void print() const override {
<<<<<<< HEAD
		std::cout << m_line << '\n'
				  << "Expect function definition or variable declaration!" << '\n'
				  << "But got: " << tokenToString(m_tok) << '\n';
=======
		std::stringstream ss;
		ss << "Expect function definition or variable declaration,";
		ss << " but got: " << tokenToString(m_tokinfo.m_tok) << std::endl;

		printErrorLine(ss.str());
>>>>>>> c45277eeed906a3d0a3c8940b1cb8c900a0cf0be
	}
};

class Warning {};
}
