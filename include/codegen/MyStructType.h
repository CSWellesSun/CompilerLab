// modifiy llvm::StructType from llvm/IR/DerivedTypes.h
#pragma once
#include "llvm/IR/DerivedTypes.h"
#include <vector>
#include <string>
#include <algorithm>

#include "common/Defs.h"
// using namespace llvm;

class MyStructType{
public:
    MyStructType() = default;
    void AddElementName(const std::string& sr) { 
        // LOG_INFO("TEST2");
        Names.push_back(sr); 
    }
    std::string getNameAtIndex(unsigned N) const { return Names.at(N); }
    unsigned findIndexofName(const std::string& name) const {
        auto finditer = std::find(Names.cbegin(), Names.cend(), name);
        if (finditer == Names.cend())
            return static_cast<unsigned>(-1);
        else
            return static_cast<unsigned>(finditer - Names.cbegin());
    }
    auto& GetStructType() { return structType; }
    auto& GetStructMemNames() { return Names; }
private:
    llvm::StructType* structType;
    std::vector<std::string> Names {};
};
