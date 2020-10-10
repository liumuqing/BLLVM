#include "IR/Instruction.hpp"
#include "IR/BasicBlock.hpp"

extern template class InstructionKind<UndefiendInstruction, UNDEF>;

void Instruction::pushToBBL(BasicBlock * bbl) {
	bbl->push_back(this->shared_from_this<Instruction>());
}
