#include "Pass/ValidateModulePass.hpp"

#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Instruction.hpp"

#include "Pass/ValidateFunctionPass.hpp"
#include "Pass/PassManager.hpp"

bool ValidateModulePass::run() {
	this->validated_ = true;
	for (auto function: *this->getTargetModule()) {
		getPassManager()->getAnalysisOf<ValidateFunctionPass>(function.get());
	}
common_exit:
	return false;
}
