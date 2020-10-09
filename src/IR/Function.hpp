#pragma once
#include "IR/Module.hpp"
#include "IR/Value.hpp"
#include "IR/AddressedItem.hpp"
#include "common.hpp"
class Module;
class BasicBlock;
class Parameter;
class Function:
	virtual public Value,
	virtual public AddressableListContainerItem<Function, Module>,
	virtual public AddressableListConatiner<BasicBlock>
{
public:
	virtual ~Function() {};

	uaddr_t getStart() const {
		return getAddress();
	}
	void setStart(uaddr_t addr) {
		setAddress(addr);
	}

	Parameter * createAndAppendParameter(size_t bitSize) {

	}

	Parameter * getParameter(size_t index) const {
		FATAL_UNLESS(index >= 0 && index < params_.size());
		return params_[index].get();
	}
protected:
	Function() {}
	Function(const Function&) = delete;
private:
	std::vector<std::shared_ptr<Parameter>> params_;

friend class ListContainerItem<Function, Module>;
};

class Parameter:
	virtual public Value,
	virtual public WithWidthMixin,
	virtual public WithParentMixin<Parameter, Function>
{

};
