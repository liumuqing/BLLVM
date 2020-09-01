#pragma once
#include <vector>
#include <map>
#include <memory>

#include "IR/Function.hpp"
#include "IR/Value.hpp"
#include "common.hpp"
class Module : Value {
	using Containter = std::map<uaddr_t, std::shared_ptr<Function>>;

public:
	virtual ~Module(){}
	std::vector<Function *> functions() {
		std::vector<Function *> retv;
		for (auto f_ptr: functions_) {
			retv.push_back(f_ptr.second.get());
		}
		return retv;
	}
	void add_function(uaddr_t addr, Function * function) {
		add_function(addr, std::dynamic_pointer_cast<Function>(function->shared_from_this()));
	}
	void add_function(uaddr_t addr, std::shared_ptr<Function> function) {
		if (functions_.find(addr) != functions_.end()) {
			WARN("two functions at same address %p", addr);
		}

		if (function->getStart() != addr) {
			WARN("function added to module, but it's start address does not match...Let's me fix it");
			function->setStart(addr);
		}

		functions_[addr] = function;
	}

protected:
	Containter functions_;

};
