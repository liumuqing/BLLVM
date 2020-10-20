#include "IR/Instruction.hpp"
#include "IR/BasicBlock.hpp"

const char * Instruction::getOpstr() const {
	FATAL_UNLESS(getOpcode() < sizeof(OpStr)/sizeof(OpStr[0]));
	return OpStr[getOpcode()];
}
const char * OpStr[OPCODE_COUNT] {
	#define HANDLE_INST(opcode) #opcode,
	#include "IR/Instruction.def"
	#undef HANDLE_INST
};

extern template class InstructionKind<UndefiendInstruction, UNDEF>;

void Instruction::pushToBBL(BasicBlock * bbl) {
	FATAL_UNLESS(not this->getParent());
	bbl->push_back(this->shared_from_this<Instruction>());
}
void Instruction::insertSelfAfter(Instruction * pos) {
	FATAL_UNLESS(not this->getParent());
	FATAL_UNLESS(pos->getParent());

	auto parent = dynamic_cast<BasicBlock *>(pos->getParent());
	FATAL_UNLESS(parent);

	parent->insertInstructionAfter(pos, this->shared_from_this<Instruction>());
}
void Instruction::insertSelfBefore(Instruction * pos) {
	FATAL_UNLESS(not this->getParent());
	FATAL_UNLESS(pos->getParent());

	auto parent = dynamic_cast<BasicBlock *>(pos->getParent());
	FATAL_UNLESS(parent);

	parent->insertInstructionBefore(pos, this->shared_from_this<Instruction>());
}
