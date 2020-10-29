#pragma once
#include <cstdint>
#include <memory>
#include <functional>
#include <compare>

#include "common.hpp"

class Module;
class Function;
class BasicBlock;

class PassManager;

class PassID {
public:
	explicit PassID(void * value) {
		value_ = reinterpret_cast<std::uintptr_t>(value);
	}
	auto operator<=> (const PassID& other) {
		return this->value_ <=> other.value_;
	}
private:
	std::uintptr_t value_;
};

class PassInfoMixin {
public:
	static PassID getClassId() {
		return PassID(&id_);
	}
private:
    static struct {} id_;
};

class Pass {
	Pass(PassManager * pm) {
		pm_ = pm;
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
