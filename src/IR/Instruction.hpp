#pragma once
#include "IR/AddressedItem.hpp"
#include "IR/User.hpp"
//#include "IR/BasicBlock.hpp"
class BasicBlock;
enum Opcode {
	NOP,
	ALLOC,
	ADD,
	SUB,
	XOR,
	OR,
	AND,
	NEG,
	NOT,
	LSL,
	LSR,
	ASR,
	SDIV,
	UDIV,
	MUL,
	UREM,
	SREM,
	CONST,
	ZX,
	SX,
	LOAD,
	STORE,
	LOAD_STACK,
	STORE_STACK,
	CMP_EQ,
	CMP_LT,
	CALL,
	CONST_INT,
	PHI,
	//BBL Terminator
	SWITCH,
	BRANCH,
	//Anything, float instruction, etc
	UNDEF,
	//Function Terminator
	RETURN,
	UNREACHABLE,
};

class Instruction:
	virtual public Value,
	virtual public WithWidthMixin,
	virtual public WithParentMixin,
	virtual public ListContainerItem<Instruction>,
	virtual public User{
protected:

#define DEFINE_CONFIG_TYPE(Name, Type) \
	class Name {\
	public:\
		explicit Name(Type const& value) {\
			value_ = value;\
		}\
		Name(const Name& other) {\
			value_ = other.value_;\
		}\
		operator Type() {\
			return value_;\
		}\
		Type getValue() const { \
			return value_; \
		}\
	private:\
		Type value_;\
	}\

public:
	DEFINE_CONFIG_TYPE(BitWidth, size_t);
	DEFINE_CONFIG_TYPE(BeginOfBasicBlock, Instruction *);
	DEFINE_CONFIG_TYPE(EndOfBasicBlock, BasicBlock *);
	DEFINE_CONFIG_TYPE(AfterInstruction, Instruction *);
	DEFINE_CONFIG_TYPE(BeforeInstruction, Instruction *);
public:
	template<typename... Args>
	void appendOperands(Value * firstOperand, Args... remainingOperands) {
		this->setNumOperands(this->getNumOperands()+1);
		this->setOperand(this->getNumOperands()-1, firstOperand);
		if (sizeof...(remainingOperands)) {
			this->appendOperands(remainingOperands...);
		}
	}

	//void setOpcode(Opcode opcode);
	virtual Opcode getOpcode() const = 0;
protected:
	Instruction(){};
	void insertSelfAfter(Instruction * pos);
	void insertSelfBefore(Instruction * pos);
	void pushToBBL(BasicBlock * bbl);


private:
	void appendOperands() const {}

	friend class BasicBlock;
	template <typename InstructionType> friend inline auto configInstruction(std::shared_ptr<InstructionType> self, Instruction::EndOfBasicBlock bbl);
	template <typename InstructionType> friend inline auto configInstruction(std::shared_ptr<InstructionType> self, Instruction::BeforeInstruction pos);
	template <typename InstructionType> friend inline auto configInstruction(std::shared_ptr<InstructionType> self, Instruction::AfterInstruction pos);
	template <typename InstructionType> friend inline auto configInstruction(std::shared_ptr<InstructionType> self, Instruction::BeginOfBasicBlock bbl);
};

template <typename InstructionType, typename ConfigValue> inline InstructionType* configInstruction(InstructionType *self, ConfigValue value) {
	auto pointer = self->template shared_from_this<InstructionType>();
	configInstruction(pointer, value.getValue());
	return self;
}
//template <typename InstructionType, typename ConfigValue> inline auto configInstruction(std::shared_ptr<InstructionType> self, ConfigValue value);
template <typename InstructionType> inline auto configInstruction(std::shared_ptr<InstructionType> self, Instruction::BitWidth bitWidth) {
	self->setBitWidth(bitWidth);
	return self;
}
template <typename InstructionType> inline auto configInstruction(std::shared_ptr<InstructionType> self, Instruction::EndOfBasicBlock bbl) {
	FATAL_UNLESS(not self->getParent());
	self->pushToBBL(bbl);
	return self.get();
}
template <typename InstructionType> inline auto configInstruction(std::shared_ptr<InstructionType> self, Instruction::AfterInstruction pos) {
	FATAL_UNLESS(not self->getParent());
	self->insertSelfAfter(pos);
	return self.get();
}
template <typename InstructionType> inline auto configInstruction(std::shared_ptr<InstructionType> self, Instruction::BeforeInstruction pos) {
	FATAL_UNLESS(not self->getParent());
	self->insertSelfBefore(pos);
	return self.get();
}

template <typename Self, Opcode opcode>
class InstructionKind: virtual public Instruction {
public:
	template<typename ArgType, typename... Args>
	static inline std::shared_ptr<Self> create(ArgType* firstOperand, Args... args) {
		auto retv = create();
		retv->appendOperands(firstOperand, args...);
		return retv;
	}
    template<typename FirstConfig, typename... Args>
    static inline auto create(const FirstConfig& oneConfig, Args... args) {
		auto retv = create(args...);
		return configInstruction(retv, oneConfig);
	}
	static auto create() {
		auto retv = std::shared_ptr<Self>(new Self());
		retv->setNumOperands(0);
		return retv;
	}
	virtual Opcode getOpcode() const override final {
		return opcode;
	}
};


class UnaryInstruction: virtual public Instruction {
};

class BinaryInstruction: virtual public Instruction {
	private:
		void makeSureAtLeastTwoOperands() {
			if (getNumOperands() < 2) {
				setNumOperands(2);
			}
		}
	public:
		Value * getLeft() const {
			FATAL_UNLESS(getNumOperands() >= 1);
			return getOperand(0);
		}
		Value * getRight() const {
			FATAL_UNLESS(getNumOperands() >= 2);
			return getOperand(1);
		}
		void setLeft(Value * operand) {
			makeSureAtLeastTwoOperands();
			setOperand(0, operand);
		}
		void setRight(Value * operand) {
			makeSureAtLeastTwoOperands();
			setOperand(1, operand);
		}
};
class UndefiendInstruction: virtual public InstructionKind<UndefiendInstruction, UNDEF> {};
class NopInstruction: virtual public InstructionKind<NopInstruction, NOP> {};

class AddInstruction:
	virtual public InstructionKind<AddInstruction, ADD>,
	virtual public BinaryInstruction {
};
class SubInstruction:
	virtual public InstructionKind<SubInstruction, SUB>,
	virtual public BinaryInstruction {
};
class MulInstruction:
	virtual public InstructionKind<MulInstruction, MUL>,
	virtual public BinaryInstruction {
};
class SRemInstruction:
	virtual public InstructionKind<SRemInstruction, SREM>,
	virtual public BinaryInstruction {
};
class URemInstruction:
	virtual public InstructionKind<URemInstruction, UREM>,
	virtual public BinaryInstruction {
};
class SDivInstruction:
	virtual public InstructionKind<SDivInstruction, SDIV>,
	virtual public BinaryInstruction {
};
class UDivInstruction:
	virtual public InstructionKind<UDivInstruction, UDIV>,
	virtual public BinaryInstruction {
};

class AndInstruction:
	virtual public InstructionKind<AndInstruction, AND>,
	virtual public BinaryInstruction {
};
class OrInstruction:
	virtual public InstructionKind<OrInstruction, OR>,
	virtual public BinaryInstruction {
};
class XorInstruction:
	virtual public InstructionKind<XorInstruction, XOR>,
	virtual public BinaryInstruction {
};

class NotInstruction:
	virtual public InstructionKind<NotInstruction, NOT>,
	virtual public UnaryInstruction{
};

class NegInstruction:
	virtual public InstructionKind<NegInstruction, NEG>,
	virtual public UnaryInstruction{
};

class UnsignedExtendInstruction:
	virtual public InstructionKind<UnsignedExtendInstruction, ZX>,
	virtual public UnaryInstruction{
};

class SignedExtendInstruction:
	virtual public InstructionKind<SignedExtendInstruction, SX>,
	virtual public UnaryInstruction{
};
	
class AllocInstruction: virtual public InstructionKind<AllocInstruction, ALLOC> {
	public:
		DEFINE_CONFIG_TYPE(VariableBitWidth, size_t);

		void setVariableBitWidth(size_t varialbeBitWidth) {
			varialbeBitWidth_ = varialbeBitWidth;
		}
		size_t getVariableBitWidth() const {
			return varialbeBitWidth_;
		}

	private:
		size_t varialbeBitWidth_;

};
inline auto configInstruction(std::shared_ptr<AllocInstruction> self, AllocInstruction::VariableBitWidth width) {
	self->setVariableBitWidth(width);
	return self;
}
class LoadInstruction: virtual public InstructionKind<LoadInstruction, LOAD> {};
