#include "codegen/CodeGen.h"
#include "lexer/TokenStream.h"
#include "parser/Parser.h"
#include "preprocess/Preprocess.h"
#include "typesystem/TypeSystem.h"
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
	cout << '\n';
	TokenStream tokenStream(preprocess);
	tokenStream.Dump();
	cout << "\n\n";
	Parser parser(tokenStream);
	parser.parse();
	parser.Dump();
	cout << '\n';
	CodeGenerator codeGenerator(parser.GetAst());
	codeGenerator.Dump();
	codeGenerator.srctollFile(input);

	/* 	After obtaining .ll file,
		using llvm-as to convert .ll to .bc (llvm bitcode)
		and use clang to convert .bc to executable file
		For example:
			llvm-as ./res/a.ll
			clang ./res/a.bc -o a.out
	*/
}