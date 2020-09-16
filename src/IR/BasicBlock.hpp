#pragma once
#include <memory>

#include "AddressedItem.hpp"
#include "common.hpp"
#include "IR/Value.hpp"
#include "IR/Function.hpp"
#include "IR/Instruction.hpp"

#include <list>
class Module;
class Function;
class Instruction;
class BasicBlock:
	virtual public Value,
	//public AddressedMixin<BasicBlock>,
	virtual public AddressedWithParentMixin<Function, BasicBlock>
{
	using Self = BasicBlock;
	using Conatiner = std::list<std::shared_ptr<Instruction>>;
public:
	/*
	static inline std::shared_ptr<BasicBlock> create() {
		std::shared_ptr<BasicBlock> retv = std::shared_ptr<BasicBlock>(new BasicBlock());
		return retv;
	}
	static inline std::shared_ptr<BasicBlock> create(uaddr_t address) {
		auto retv = create();
		retv->setAddress(address);
		return retv;
	}
	static inline std::shared_ptr<BasicBlock> create(Function *parent, uaddr_t address) {
		auto retv = create(address);
		retv->setAddress(address);
		parent->addAddressedItem(retv);
		return retv;
	}
	static inline std::shared_ptr<BasicBlock> create(std::weak_ptr<Function> parent, uaddr_t address) {
		Function * p = parent.lock().get();
		return create(p, address);
	}
	*/
	virtual ~BasicBlock() {};
	friend class AddressedMixin<BasicBlock>;
	friend class AddressedWithParentMixin<Function, BasicBlock>;
protected:
	BasicBlock() {};
	BasicBlock(const Function&) = delete;
};
