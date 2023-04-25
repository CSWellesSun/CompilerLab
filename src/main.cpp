#include "lexer/TokenStream.h"
#include "parser/Parser.h"
#include <iostream>

using namespace std;
using namespace minisolc;

int main(int argc, const char* argv[]) {
	auto input = argv[1];
	TokenStream tokenStream(input);
	tokenStream.dump();
	cout << '\n';
	Parser parser(tokenStream);
	parser.parse();
	parser.Dump();
}