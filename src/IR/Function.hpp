#pragma once
#include "IR/Value.hpp"
#include "IR/AddressedItem.hpp"
#include "common.hpp"
class Module;
class BasicBlock;
class Function:
	public Value,
	public AddressedWithParentMixin<Module, Function>,
	public AddressedConatinerMixin<BasicBlock>
{
public:
	Function() {}
	Function(uaddr_t address) {
		setAddress(address);
	}

	Function(const Function&) = delete;
	virtual ~Function() {};

	uaddr_t getStart() const {
		return getAddress();
	}
	void setStart(uaddr_t addr) {
		setAddress(addr);
	}
};
