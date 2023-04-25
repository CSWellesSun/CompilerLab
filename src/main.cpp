#include <iostream>
#include "lexer/TokenStream.h"
#include "parser/Parser.h"
using namespace std;
using namespace minisolc;

int main(int argc, const char* argv[]) {
	auto input = argv[1];
	CharStream charStream(std::string(input), CharStream::File);
	TokenStream tokenStream(charStream);
	tokenStream.dump();
	cout << endl;
	Parser parser(tokenStream);
	parser.parse();
	parser.Dump();
}