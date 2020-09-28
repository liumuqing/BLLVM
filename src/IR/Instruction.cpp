#include "IR/Instruction.hpp"
#include "IR/BasicBlock.hpp"
std::shared_ptr<Instruction> Instruction::create(BasicBlock * bbl, Opcode opcode) {
	auto retv = create(opcode);
	bbl->push_back(retv);
	return retv;
}
