#pragma once
#include <cstdint>
#include <memory>
#include <functional>

#include "common.hpp"
#include "Utils/Object.hpp"

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

class Pass: virtual public Object {
public:
	Pass() {}
	void initialize(PassManager * pm);
	virtual ~Pass() = default;
	virtual PassID getPassId() const = 0;
	virtual bool run() = 0;
protected:
	std::weak_ptr<PassManager> pm_;
	PassManager * getPassManager() const {
		FATAL_UNLESS(not pm_.expired());
		auto retv = pm_.lock().get();
		FATAL_UNLESS(retv);
		return retv;
	}
};

class PassInfoMixin: virtual public Pass {
public:
	virtual PassID getPassId() const override final {
		return PassID(&ID);
	}
    static struct {} ID;
};

class Analysis: virtual public Pass {
public:
	virtual ~Analysis() = default;
};

class Transformation: virtual public Pass {
public:
	virtual ~Transformation() = default;
};

template <typename IRUintT> class IRUnitPass: virtual public Pass {
public:
	IRUnitPass() {}
	void initialize(IRUintT * target, PassManager * passManager) {
		FATAL_UNLESS(target);

		Pass::initialize(passManager);
		target_ = target->template weak_from_this<IRUintT>();
	}
	IRUintT * getTarget() const {
		FATAL_UNLESS(not target_.expired());
		auto retv = target_.lock().get();
		FATAL_UNLESS(retv);
	}
	virtual ~IRUnitPass() = default;
private:
	std::weak_ptr<IRUintT> target_;
};

typedef IRUnitPass<Function> FunctionPass;
typedef IRUnitPass<Module> ModulePass;
