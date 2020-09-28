#include "IR/BasicBlock.hpp"
#include "common.hpp"
std::shared_ptr<Instruction> BasicBlock::removeInstruction(Instruction * inst) {
	auto it = getIteratorOfInstruction(inst);
	auto retv = *it;
	instructions_.erase(it);
	retv->setParent(nullptr);
	return retv;
}
void BasicBlock::insertInstructionAt(Conatiner::iterator it, std::shared_ptr<Instruction> inserted) {
	if (inserted->parent_) {
		FATAL("inst that to be inserted, already blongs to an bbl");
	}
	this->instructions_.insert(it, inserted);
	inserted->parent_ = this;
}
void BasicBlock::insertInstructionAfter(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted) {
	auto it = getIteratorOfInstruction(insertPoint);
	it++;
	insertInstructionAt(it, inserted);
}
void BasicBlock::insertInstructionBefore(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted) {
	auto it = getIteratorOfInstruction(insertPoint);
	insertInstructionAt(it, inserted);
}


BasicBlock::Conatiner::iterator BasicBlock::getIteratorOfInstruction(const Instruction * inst) {
	auto it = std::find(instructions_.begin(), instructions_.end(), inst->shared_from_this());
	if (it == instructions_.end()) {
		FATAL("cannot find insertpoint in bbl");
	}
	return it;
}
