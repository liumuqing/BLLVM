#include "Pass/Pass.hpp"

#include "Pass/PassManager.hpp"

void Pass::initialize(PassManager * pm) {
	this->pm_ = pm->weak_from_this<PassManager>();
}
