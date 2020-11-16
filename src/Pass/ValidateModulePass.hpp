#include "Pass/Pass.hpp"
#include "IR/Module.hpp"

class ValidateModulePass: virtual public ModulePass {
public:
	bool run() override final;
	bool isValidated() const {
		return validated_;
	}
private:
	bool validated_ = false;
};
