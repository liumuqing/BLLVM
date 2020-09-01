#pragma once
#include <cstddef> //size_t
#include <memory>
#include <vector>

#include "IR/Value.hpp"
#include "common.hpp"

#include "IR/Use.hpp"

class User : Value {
public:
	virtual void replaceUsesOfWith(Value *From, Value *To) {
		for (auto i = 0; i < getNumOperands(); i++) {

			// when you are iterating in operands, those operands must still alive...

			if (getOperand(i) == From) {
				setOperand(i, To);
			}

		}
	}
	virtual std::size_t getNumOperands() const {
		return operands.size();
	}
	virtual void setNumOperands(size_t size) {
		while (operands.size() > size) {
			operands.pop_back();
		}
		while (operands.size() < size) {
			operands.push_back(Use(this));
		}
	}
	virtual Value* getOperand(size_t index) {
		return operands[index].getValue();
	}
	virtual void setOperand(size_t index, Value *value) {
		operands[index].setValue(value);
	}
	virtual ~User(){}
protected:
	std::vector<Use> operands;
};
