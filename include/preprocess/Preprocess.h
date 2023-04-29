#pragma once

#include <string>
#include <vector>

namespace minisolc {

struct PreprocessFile {
	std::string m_filename;
	std::string m_charstream;
};

class Preprocess {
public:
	Preprocess(const std::string& filename): m_filename(filename){
        preprocess();
    };
	void preprocess();
	std::string getFilename() const { return m_filename; };
	std::vector<PreprocessFile> getFiles() const { return m_files; }
	void Dump() const;

private:
    std::string getRelativePath(const std::string& path) const;

	std::string m_filename;
	std::vector<PreprocessFile> m_files;
};

}
