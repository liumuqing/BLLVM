#pragma once
#include <cstdint>
#include <memory>

#include <common.hpp>

class Module;
class Function;
class BasicBlock;

class PassManager;

template <typename Self> class PassInfoInfoMixin {
public:
    // used as &Pass::ID, to represet a class
    static uintptr_t getClassId() {
        return static_cast<uintptr_t>(&ID);
    }
private:
    static struct {} ID;
};

class Pass {
	Pass(PassManager * pm) {
		pm_ = pm->;
	};
	bool run(Module * module, PassManager * pm);
	bool run(Function * function, PassManager * pm);
	bool run(BasicBlock * basicBlock, PassManager * pm);
	virtual ~Pass() = default;
protected:
	std::weak_ptr<PassManager> pm_;
	PassManager * getPassManager() const {
		FATAL_UNLESS(not pm_.expired());
		auto retv = pm_.lock().get();
		FATAL_UNLESS(retv);
		return retv;
	}
};
class Analysis {
	Pass() {};
	bool run(Module * module, PassManager * pm);
	bool run(Function * function, PassManager * pm);
	bool run(BasicBlock * basicBlock, PassManager * pm);
	virtual ~Pass() = default;
}

class ModulePass {
}
