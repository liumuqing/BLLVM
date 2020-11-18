#include "Pass/ValidateFunctionPass.hpp"

#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Instruction.hpp"

bool ValidateFunctionPass::run() {
	this->validated_ = true;
	for (auto bbl: *this->getTargetFunction()) {
		//check if all bbl terminated by an terminator
		bool terminatorSeen = false;
		for (auto inst: *bbl) {

			if (std::dynamic_pointer_cast<TerminatorInstruction>(inst)) {
				if (terminatorSeen) {
					goto not_valid_exit;
				}
				terminatorSeen = true;
			}
			else {
				// if we see a non-terminator instruction after terminatoer instruction
				if (terminatorSeen) {
					goto not_valid_exit;
				}
			}
		}
	}
common_exit:
	return false;
not_valid_exit:
	this->validated_ = false;
	return false;
}
