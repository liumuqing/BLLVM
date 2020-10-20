#include <memory>
#include "Constant.hpp"

ConstantInt * ConstantInt::create(Module * parent, size_t bitWidth, uint64_t value) {
	auto r = parent->getConstantInt(bitWidth, value, false);
	if (r) {
		return r;
	}
	auto retv = std::shared_ptr<ConstantInt>(new ConstantInt());
	retv->setBitWidth(bitWidth);
	retv->value_ = value;
	INFO("%d\n", bitWidth);
	FATAL_UNLESS(retv->getUnsignedValue() == value);
	parent->addConstantInt(retv);
	return retv.get();
}

uint64_t ConstantInt::getUnsignedValue() {
	if (getBitWidth() == 64) {
		return this->value_ & (uint64_t)(-1);
	}
	return this->value_ & (((uint64_t)1 << getBitWidth())-1);
}

void ConstantInt::setBitWidth(size_t bitWidth) {
	FATAL_UNLESS(not this->getParent());
	return WithWidthMixin::setBitWidth(bitWidth);
}
