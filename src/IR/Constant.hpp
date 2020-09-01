#include "IR/User.hpp"
class ConstantInt: public Value {

public:
	ConstantInt(size_t bitWidth, uint64_t value);
	uint64_t getUnsignedValue();
	//int64_t getSignedValue(); TODO

private:
	uint64_t value_;
	size_t bitWidth_;
};
