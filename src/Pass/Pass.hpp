#pragma once
#include <cstdint>
#include <memory>
#include <functional>

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
	auto operator== (const PassID& other) {
		return this->value_ == other.value_;
	}
	auto operator< (const PassID& other) {
		return this->value_ < other.value_;
	}
	auto operator> (const PassID& other) {
		return this->value_ > other.value_;
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
public:
	Pass(PassManager * pm);
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

class Analysis: virtual public Pass {
	virtual ~Analysis() = default;
};

class Transformation: virtual public Pass {
	virtual ~Transformation() = default;
};


class ModulePass: virtual public Pass {
	virtual bool run(Module * module, PassManager pm) = 0;
};

class FunctionPass: virtual public Pass {
	virtual bool run(Function * function, PassManager pm) = 0;
};
