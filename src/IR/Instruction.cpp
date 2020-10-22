#include "IR/Instruction.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Constant.hpp"

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
/*
 * return the operand, if it's the only immediate operand.
 * otherwise return nullptr
 */
ConstantInt * CompareInstruction::getUniqueImmediateOperand() const {
	auto left = dynamic_cast<ConstantInt *>(getLeft());
	auto right = dynamic_cast<ConstantInt *>(getRight());
	if (left and not right) {
		return left;
	}
	if (right and not left) {
		return right;
	}
	return nullptr;
}
/*
 * return the operand, if it's the only non-immediate operand.
 * otherwise return nullptr
 */
Value * CompareInstruction::getUniqueNonImmediateOperand() const {
	auto left = dynamic_cast<ConstantInt *>(getLeft());
	auto right = dynamic_cast<ConstantInt *>(getRight());
	if (left and not right) {
		return getRight();
	}
	if (right and not left) {
		return getLeft();
	}
	return nullptr;
}

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
