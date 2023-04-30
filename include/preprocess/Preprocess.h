#pragma once

#include "CharStream.h"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace minisolc {

class Preprocess {
public:
	Preprocess(std::filesystem::path filePath) { preprocess(filePath); };
	void preprocess(std::filesystem::path filePath, std::shared_ptr<Line> includeLine = nullptr);
	CharStream source() const { return m_stream; }
	void Dump() const { m_stream.Dump(); };

private:
	std::filesystem::path getRelativePath(std::filesystem::path path) const;
	void processInclude(const std::string& line, std::filesystem::path parentPath, std::shared_ptr<Line> includeLine);

	CharStream m_stream;
};

}
