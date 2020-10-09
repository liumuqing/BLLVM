#include <memory>
#include <optional>

#include "IR/User.hpp"
#include "IR/Module.hpp"
class Module;
class ConstantInt:
	virtual public Value,
	virtual public WithParentMixin<ConstantInt, Module>,
	virtual public WithWidthMixin
{

public:
	static ConstantInt * create(Module * parent, size_t bitWidth, uint64_t value);
	ConstantInt(const ConstantInt&) = delete;
	ConstantInt(ConstantInt&) = delete;
	uint64_t getUnsignedValue();

	// override the method, check if parent is set.
	// if so, panic....
	virtual void setBitWidth(size_t bitWidth) override;

private:
	uint64_t value_ = 0;
	std::optional<std::weak_ptr<Module>> parent_ = std::nullopt;

	ConstantInt() {};

friend class Module;
};
