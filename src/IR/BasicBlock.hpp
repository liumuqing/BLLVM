#pragma once
#include <memory>

#include "AddressedItem.hpp"
#include "common.hpp"
#include "IR/Value.hpp"
#include "IR/Function.hpp"
#include "IR/Instruction.hpp"

#include "common.hpp"

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
	void insertInstructionAfter(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted);
	void insertInstructionBefore(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted);
	void insertInstructionAtBegin(std::shared_ptr<Instruction> inserted) {
		insertInstructionAt(instructions_.begin(), inserted);
	}
	void insertInstructionAtEnd(std::shared_ptr<Instruction> inserted) {
		insertInstructionAt(instructions_.end(), inserted);
	}
	virtual ~BasicBlock() {};
	friend class AddressedMixin<BasicBlock>;
	friend class AddressedWithParentMixin<Function, BasicBlock>;
protected:
	BasicBlock() {};
	BasicBlock(const Function&) = delete;

private:
	Conatiner instructions_;
    void insertInstructionAt(Conatiner::iterator it, std::shared_ptr<Instruction> inserted);
};
