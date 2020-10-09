#include "IR/Module.hpp"
#include "IR/Function.hpp"
#include "IR/Constant.hpp"
ConstantInt* Module::getConstantInt(size_t bitWidth, uint64_t unsignedValue) {
	auto end = constantIntMap_.end();
	auto iter = constantIntMap_.find(std::make_pair(bitWidth, unsignedValue));
	if (iter != end) {
		FATAL_UNLESS(iter->second->getUnsignedValue() == unsignedValue);
		FATAL_UNLESS(iter->second->getBitWidth() == bitWidth);
		return iter->second.get();
	}
	return nullptr;
}
void Module::addConstantInt(std::shared_ptr<ConstantInt> ci) {

	FATAL_UNLESS(not ci->getParent());
	FATAL_UNLESS(not constantIntMap_.contains({ci->getBitWidth(), ci->getUnsignedValue()}));
	ci->setParent(this);
	constantIntMap_[std::make_pair(ci->getBitWidth(), ci->getUnsignedValue())] = ci;
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
