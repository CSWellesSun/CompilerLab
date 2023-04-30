#include "lexer/TokenStream.h"
#include "parser/Parser.h"
#include "preprocess/Preprocess.h"
#include <iostream>

#ifdef _WIN32
#include "windows.h"
#endif

using namespace std;
using namespace minisolc;

int main(int argc, const char* argv[]) {
#ifdef _WIN32
	SetConsoleOutputCP(65001);
#endif
	auto input = argv[1];
	Preprocess preprocess(input);
	preprocess.Dump();
	cout << "\n";
	TokenStream tokenStream(preprocess);
	tokenStream.Dump();
	cout << "\n\n";
	Parser parser(tokenStream);
	parser.parse();
	parser.Dump();
}