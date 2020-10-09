#include "IR/Instruction.hpp"
#include "IR/BasicBlock.hpp"
Instruction* Instruction::create(BasicBlock * bbl, Opcode opcode) {
	auto retv = create(opcode);
	bbl->push_back(retv);
	return retv.get();
}
void Instruction::setOpcode(Opcode opcode) {
	this->opcode_ = opcode;
}
Opcode Instruction::getOpcode() const {
	return this->opcode_;
}
