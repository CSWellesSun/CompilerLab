#include "preprocess/Preprocess.h"
#include "common/Defs.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>


using namespace minisolc;

void Preprocess::preprocess(std::filesystem::path filePath, std::shared_ptr<Line> includeLine) {
	std::filesystem::path dirPath = filePath.parent_path();
	std::ifstream file(filePath);
	std::string content;

	if (file.is_open()) {
		/* load input file stream into string */
		content = std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
		file.close();
	} else {
		LOG_WARNING("Program does not open file %s", filePath.string().c_str());
		return;
	}

	size_t lineNumber = 0;
	std::string line;
	std::stringstream ss(content);
	while (getline(ss, line, '\n')) {
		std::shared_ptr<Line> linePtr
			= std::make_shared<Line>(line + "\n", ++lineNumber, getRelativePath(filePath).string(), includeLine);
		if (line.find("#include") == 0) {
			processInclude(line, dirPath, linePtr);
		} else if (line.find("#define") == 0) {
			processDefine(line);
		} else {
			for (const auto& define: m_defines) {
				size_t pos = linePtr->source.find(define.first);
				while (pos != std::string::npos) {
					linePtr->source.replace(pos, define.first.length(), define.second);
					pos = linePtr->source.find(define.first, pos + define.second.length());
				}
			}
			m_stream.push_back(linePtr);
		}
	}

	LOG_INFO("Preprocess Succeeds.");
}

std::filesystem::path Preprocess::getRelativePath(std::filesystem::path path) const {
	// 获取当前程序的路径
	std::filesystem::path currentPath = std::filesystem::current_path();

	// 计算相对路径
	std::filesystem::path relativePath = std::filesystem::relative(path, currentPath);

	return relativePath;
}

void Preprocess::processInclude(
	const std::string& line, std::filesystem::path parentPath, std::shared_ptr<Line> includeLine) {
	size_t pos = line.find_first_of('"');
	size_t rightpos = line.find_last_of('"');
	if (pos == std::string::npos || rightpos == std::string::npos) {
		LOG_WARNING("Invalid include directive %s", line.c_str());
		return;
	}
	std::string filename = line.substr(pos + 1, rightpos - pos - 1);
	std::filesystem::path filePath = parentPath / filename;
	preprocess(filePath, includeLine);
}

void Preprocess::processDefine(const std::string& line) {
	std::stringstream ss(line);
	std::string keyword, key, value;

	ss >> keyword >> key >> value;

	if (keyword == "#define" && !key.empty() && !value.empty()) {
		m_defines[key] = value;
	} else {
		LOG_WARNING("Invalid #define format %s.", line.c_str());
	}
}
