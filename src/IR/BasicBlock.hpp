#pragma once
#include <memory>

#include "AddressedItem.hpp"
#include "common.hpp"
#include "IR/Value.hpp"
#include "IR/Instruction.hpp"

#include "common.hpp"

#include <list>
class Module;
class Function;
class Instruction;
class BasicBlock:
	virtual public Value,
	virtual public AddressableListContainerItem<BasicBlock>,
	virtual public ListConatiner<Instruction>
{
	using Self = BasicBlock;
	using Conatiner = ListConatiner<Instruction>;
public:
	void insertInstructionAfter(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted);
	void insertInstructionBefore(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted);
	void insertInstructionAtBegin(std::shared_ptr<Instruction> inserted) {
		insertInstructionAt(instructions_.begin(), inserted);
	}
	void insertInstructionAtEnd(std::shared_ptr<Instruction> inserted) {
		insertInstructionAt(instructions_.end(), inserted);
	}
	std::shared_ptr<Instruction> removeInstruction(Instruction * inst);
	virtual ~BasicBlock() {};
	//friend class AddressedMixin<BasicBlock>;
	friend class ListContainerItem<BasicBlock>;
	//friend class AddressableListContainerItem<BasicBlock, Function>;
protected:
	BasicBlock() {};
	BasicBlock(const Function&) = delete;

private:
	Conatiner instructions_;
    void insertInstructionAt(Conatiner::iterator it, std::shared_ptr<Instruction> inserted);
};
