#include "IR/Module.hpp"
#include "IR/Function.hpp"
#include "IR/Constant.hpp"
#include "IR/Symbol.hpp"
ConstantInt* Module::getConstantInt(size_t bitWidth, uint64_t unsignedValue, bool autoCreate) {
    auto end = constantIntMap_.end();
    auto iter = constantIntMap_.find(std::make_pair(bitWidth, unsignedValue));
    if (iter != end) {
        FATAL_UNLESS(iter->second->getUnsignedValue() == unsignedValue);
        FATAL_UNLESS(iter->second->getBitWidth() == bitWidth);
        return iter->second.get();
    }
    if (not autoCreate) {
        return nullptr;
    }

    auto retv = ConstantInt::create(this, bitWidth, unsignedValue);
    FATAL_UNLESS(constantIntMap_.contains(std::make_pair(bitWidth, unsignedValue)));
    FATAL_UNLESS(constantIntMap_[std::make_pair(bitWidth, unsignedValue)].get() == retv);
    return retv;
}
void Module::addConstantInt(std::shared_ptr<ConstantInt> ci) {

    FATAL_UNLESS(not ci->getParent());
    FATAL_UNLESS(not constantIntMap_.contains({ci->getBitWidth(), ci->getUnsignedValue()}));
    ci->setParent(this);
    constantIntMap_[std::make_pair(ci->getBitWidth(), ci->getUnsignedValue())] = ci;
}

void Module::addSymbol(std::shared_ptr<Symbol> symbol) {

    FATAL_UNLESS(not symbol->getParent());
    FATAL_UNLESS(not symbolMap_.contains(symbol->getSymbolName()));
    symbol->setParent(this);
    symbolMap_[symbol->getSymbolName()] = symbol;
}
Symbol* Module::getSymbol(const char * symbolName, bool autoCreate) {
    auto end = symbolMap_.end();
    auto iter = symbolMap_.find(symbolName);
    if (iter != end) {
        FATAL_UNLESS(iter->second->getSymbolName() == symbolName);
        return iter->second.get();
    }
    if (not autoCreate) {
        return nullptr;
    }

    auto retv = Symbol::create(this, symbolName);
    FATAL_UNLESS(symbolMap_.contains(symbolName));
    FATAL_UNLESS(symbolMap_[symbolName].get() == retv);
    return retv;
}

void Module::addFunction(uaddr_t addr, Function * function) {
    addFunction(addr, std::dynamic_pointer_cast<Function>(function->shared_from_this()));
}

void Module::removeFunction(Function * function) {
    FATAL_UNLESS(function);
    FATAL_UNLESS(function->hasSetAddress());
    FATAL_UNLESS(function->getParent() == this);
    popItemByAddress(function->getAddress());
}
void Module::addFunction(uaddr_t addr, std::shared_ptr<Function> function) {
    push_back(function);
}
