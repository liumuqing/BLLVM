#pragma once
#include <vector>
#include <map>
#include <memory>

#include "IR/Function.hpp"
#include "IR/Value.hpp"
#include "IR/AddressedItem.hpp"
#include "common.hpp"
class Function;
class Module : public Value,
	public AddressedConatinerMixin<Function>
{
	using Containter = std::map<uaddr_t, std::shared_ptr<Function>>;

public:
	virtual ~Module(){}
	std::vector<Function *> functions() {
		return children();
	}
	void addFunction(uaddr_t addr, Function * function) {
		addFunction(addr, std::dynamic_pointer_cast<Function>(function->shared_from_this()));
	}
	void addFunction(uaddr_t addr, std::shared_ptr<Function> function) {
		addAddressedItem(addr, function);
	}
	void removeFunction(Function * function) {
		assert(function);
		assert(function->hasSetAddress());
		assert(function->getParent() == this);
		popItemByAddress(function->getAddress());
	}
};
