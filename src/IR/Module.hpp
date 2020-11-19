#pragma once
#include <vector>
#include <utility>
#include <map>
#include <unordered_map>
#include <memory>

#include "IR/Value.hpp"
#include "IR/AddressedItem.hpp"
#include "common.hpp"
class Function;
class ConstantInt;
class Symbol;
class Module : virtual public Value,
    virtual public AddressableListConatiner<Function>
{
    using Containter = std::map<uaddr_t, std::shared_ptr<Function>>;

public:
    virtual ~Module(){}
    void addFunction(uaddr_t addr, Function * function);
    void addFunction(uaddr_t addr, std::shared_ptr<Function> function);
    void removeFunction(Function * function);
    ConstantInt* getConstantInt(size_t bitWidth, uint64_t unsignedValue, bool autoCreate=true);
    Symbol* getSymbol(const char *, bool autoCreate=true);
private:
    void addConstantInt(std::shared_ptr<ConstantInt> ci);
    void addSymbol(std::shared_ptr<Symbol> symbol);
    std::map<std::pair<size_t, uint64_t>, std::shared_ptr<ConstantInt>> constantIntMap_;
	std::map<std::string, std::shared_ptr<Symbol>> symbolMap_;

friend class ConstantInt;
friend class Symbol;

};
