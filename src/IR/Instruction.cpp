#include "IR/Instruction.hpp"
std::shared_ptr<Instruction> Instruction::removeFromParent() {
	if (!parent_) {
		FATAL("cannot removeFromParent");
	}
	auto retv = parent_->removeInstruction(this);
	assert(retv.get() == this);
	return retv;
}
