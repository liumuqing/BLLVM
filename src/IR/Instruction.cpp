#include "IR/Instruction.hpp"
#include "IR/BasicBlock.hpp"

extern template class InstructionKind<UndefiendInstruction, UNDEF>;

void Instruction::pushToBBL(BasicBlock * bbl) {
	FATAL_UNLESS(not this->getParent());
	bbl->push_back(this->shared_from_this<Instruction>());
}
void Instruction::insertSelfAfter(Instruction * pos) {
	FATAL_UNLESS(not this->getParent());
	auto parent = pos->getParent();
	parent->insertInstructionAfter(pos, this->shared_from_this<Instruction>());
}
void Instruction::insertSelfBefore(Instruction * pos) {
	FATAL_UNLESS(not this->getParent());
	auto parent = pos->getParent();
	parent->insertInstructionBefore(pos, this->shared_from_this<Instruction>());
}
