#include "preprocess/Preprocess.h"
#include "common/Defs.h"
#include <filesystem>
#include <fstream>
#include <iostream>


using namespace minisolc;

void Preprocess::preprocess() {
	std::ifstream file(m_filename);

	std::string content;
	if (file.is_open()) {
		/* load input file stream into string */
		content = std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
		file.close();
	} else {
		LOG_WARNING("Program does not open file %s", m_filename.c_str());
		return;
	}
	m_files.push_back({getRelativePath(m_filename), content});
}

std::string Preprocess::getRelativePath(const std::string& path) const {
	// 获取当前程序的路径
	std::filesystem::path currentPath = std::filesystem::current_path();

	// 计算相对路径
	std::filesystem::path relativePath = std::filesystem::relative(path, currentPath);

	return relativePath.string();
}