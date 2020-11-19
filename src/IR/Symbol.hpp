#pragma once

#include <memory>
#include <optional>

#include "IR/User.hpp"
#include "IR/Module.hpp"
#include "IR/AddressedItem.hpp"
class Module;
class Symbol:
    virtual public Value,
    virtual public WithParentMixin,
    virtual public WithWidthMixin
{

public:
    static Symbol* create(Module * parent, const char * name);
    Symbol(const Symbol&) = delete;
    Symbol(Symbol&) = delete;
	std::string getSymbolName() const;

    // override the method, check if parent is set.
    // if so, panic....

private:
	std::string symbolName_;
    std::optional<std::weak_ptr<Module>> parent_ = std::nullopt;

    Symbol() {};

friend class Module;
};
