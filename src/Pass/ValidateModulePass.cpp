#include "Pass/ValidateModulePass.hpp"

#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Instruction.hpp"

bool ValidateModulePass::run() {
	this->validated_ = true;
	for (auto function: *this->getTargetModule()) {
		for (auto bbl: *function) {
			//check if all bbl terminated by an terminator
			bool terminatorSeen = false;
			for (auto inst: *bbl) {
				
				if (std::dynamic_pointer_cast<TerminatorInstruction>(inst)) {
					if (terminatorSeen) {
						this->validated_ = false;
						goto common_exit;
					}
					terminatorSeen = true;
				}
				else {
					// if we see a non-terminator instruction after terminatoer instruction
					if (terminatorSeen) {
						this->validated_ = false;
						goto common_exit;
					}
				}
			}
		}
	}
common_exit:
	return false;
}
