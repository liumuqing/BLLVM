#include "Constant.hpp"
ConstantInt::ConstantInt(size_t bitWidth, uint64_t value){
	assert(bitWidth == 1 || bitWidth == 8 || bitWidth == 16 || bitWidth == 32 || bitWidth == 64);
	this->bitWidth_ = bitWidth;
	this->value_ = value;
}
uint64_t ConstantInt::getUnsignedValue() {
	return this->value_ & (((uint64_t)1 << bitWidth_)-1);
}
