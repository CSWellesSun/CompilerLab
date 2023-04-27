#pragma once

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "lexer/Token.h"

namespace minisolc {

class Error: public std::exception {
public:
	Error(const std::string& msg): m_msg(msg) {}
	virtual const char* what() const throw() { return m_msg.c_str(); }
	virtual void print() const = 0;

private:
	std::string m_msg;
};

class ParseError: public Error {
public:
	ParseError(const std::string& line, Token tok): Error("Parse Error"), m_line(line), m_tok(tok) {}
	void print() const {};

protected:
	std::string m_line;
	Token m_tok;
};

class UnexpectedToken: public ParseError {
public:
	UnexpectedToken(const std::string& line, Token tok, Token expectTok)
		: ParseError(line, tok) {
        m_expectTok.push_back(expectTok);
    }
	UnexpectedToken(const std::string& line, Token tok, bool (*func)(Token))
		: ParseError(line, tok) {
        for (int i = 0; i < (int)Token::NUM_TOKENS; i++) {
            if (func((Token)i)) {
                m_expectTok.push_back((Token)i);
            }
        }
    }

	void print() const override {
		std::cout << m_line << std::endl;
		std::cout << "Expect token: ";
        for (auto tok : m_expectTok) {
            std::cout << tokenToString(tok) << " ";
        } 
        std::cout << std::endl;
		std::cout << "But got: " << tokenToString(m_tok) << std::endl;
	}

private:
	std::vector<Token> m_expectTok;
};

class ContractDefinitionParseError: public ParseError {
public:
	ContractDefinitionParseError(const std::string& line, Token tok): ParseError(line, tok) {}
	void print() const override {
		std::cout << m_line << std::endl;
		std::cout << "Expect function definition or variable declaration!" << std::endl;
		std::cout << "But got: " << tokenToString(m_tok) << std::endl;
	}
};

class Warning {};
}
