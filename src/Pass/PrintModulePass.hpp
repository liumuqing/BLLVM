#include "Pass/Pass.hpp"

#include "IR/Module.hpp"
class PrintModulePass:
	virtual public ModulePass,
	public PassInfoMixin<PrintModulePass> {
public:
	PrintModulePass() {}
	bool run() override final;
};
