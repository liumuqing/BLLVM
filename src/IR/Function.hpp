#pragma once
//#include "IR/Module.hpp"
#include "IR/Value.hpp"
#include "IR/AddressedItem.hpp"
#include "common.hpp"
class Module;
class BasicBlock;
class Function:
	virtual public Value,
	//public AddressedMixin<Function>,
	virtual public AddressedWithParentMixin<Module, Function>,
	virtual public AddressedConatinerMixin<Function, BasicBlock>
{
public:
	virtual ~Function() {};

	uaddr_t getStart() const {
		return getAddress();
	}
	void setStart(uaddr_t addr) {
		setAddress(addr);
	}
protected:
	Function() {}
	Function(const Function&) = delete;
friend class AddressedWithParentMixin<Module, Function>;
};
