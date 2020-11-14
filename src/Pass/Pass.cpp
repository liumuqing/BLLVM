#include "Pass/Pass.hpp"

#include "Pass/PassManager.hpp"

Pass::Pass(PassManager * pm) {
	this->pm_ = pm->weak_from_this<PassManager>();
}
