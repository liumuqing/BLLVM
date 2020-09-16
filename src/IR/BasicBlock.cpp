#include "IR/BasicBlock.hpp"
#include "common.hpp"
void BasicBlock::insertInstructionAt(Conatiner::iterator it, std::shared_ptr<Instruction> inserted) {
	if (inserted->parent_) {
		FATAL("inst that to be inserted, already blongs to an bbl");
	}
	this->instructions_.insert(it, inserted);
	inserted->parent_ = this;
}
void BasicBlock::insertInstructionAfter(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted) {
	auto it = std::find(instructions_.begin(), instructions_.end(), inserted->shared_from_this());
	if (it == instructions_.end()) {
		FATAL("cannot find insertpoint in bbl");
	}
	it++;
	insertInstructionAt(it, inserted);
}
void BasicBlock::insertInstructionBefore(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted) {
	auto it = std::find(instructions_.begin(), instructions_.end(), inserted->shared_from_this());
	if (it == instructions_.end()) {
		FATAL("cannot find insertpoint in bbl");
	}
	insertInstructionAt(it, inserted);
}
