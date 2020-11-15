#include "Pass/Pass.hpp"

#include "IR/Module.hpp"
class PrintModulePass:
	virtual public ModulePass,
	public PassInfoMixin {
public:
	PrintModulePass() {}
	bool run() override; 
};
