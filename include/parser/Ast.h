#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// 所有 AST 的基类
class BaseAST {
public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
};

// SourceUnit 是 BaseAST
class SourceUnitAST : public BaseAST {
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> contract_def;

  void Dump() const override {
    std::cout << "SourceUnitAST { ";
    contract_def->Dump();
    std::cout << " }";
  }
};

class ContractDefinitionAST : public BaseAST {
public:
  // 用智能指针管理对象
  std::string ident;
  std::vector<std::unique_ptr<BaseAST>> contract_parts;

  void Dump() const override {
    std::cout << "ContractDefinitionAST { ";
    std::cout << ident << ", ";
    for (auto &contract_part : contract_parts) {
      contract_part->Dump();
      std::cout << ", ";
    }
    std::cout << " }";
  }
};

class ContractPartAST : public BaseAST {
public:
  // 用智能指针管理对象
  // StateVariableDeclarationAST / FuncDefAST
  std::unique_ptr<BaseAST> child;

  void Dump() const override {
    std::cout << "ContractPartAST { ";
    child->Dump();
    std::cout << " }";
  }
};

class StateVariableDeclarationAST : public BaseAST {
public:
  std::string type;
  std::string ident;
  std::unique_ptr<BaseAST> expr;

  void Dump() const override {}
};

// FuncDef 也是 BaseAST
class FunctionDefinitionAST : public BaseAST {
public:
  std::string ident;
  std::unique_ptr<BaseAST> param_list;
  std::vector<std::string> state_muts;
  std::string return_type;
  std::unique_ptr<BaseAST> block;

  void Dump() const override {
    std::cout << "FuncDefAST { ";
    std::cout << ident << ", ";
    param_list->Dump();
    for (auto &state_mut : state_muts) {
      std::cout << state_mut << ", ";
    }
    std::cout << return_type << ", ";
    block->Dump();
    std::cout << " }";
  }
};

class ParameterListAST : public BaseAST {
public:
  std::vector<std::pair<std::string, std::string>> params;

  void Dump() const override {}
};

/// TODO:
// ...
class FuncTypeAST : public BaseAST {
public:
  std::string type;

  void Dump() const override { std::cout << "FuncTypeAST { " << type << " }"; }
};

class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override {
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout << " }";
  }
};

class StmtAST : public BaseAST {
public:
  int val;

  void Dump() const override { std::cout << "StmtAST { " << val << " }"; }
};