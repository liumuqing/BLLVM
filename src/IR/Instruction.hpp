#pragma once
#include "IR/AddressedItem.hpp"
#include "IR/User.hpp"
#include "IR/BasicBlock.hpp"
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
	DIV,
	MUL,
	MOD,
	SMUL,
	SMOD,
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
	virtual public WithWidthMixin,
	virtual public WithParentMixin<BasicBlock>,
	virtual public ListContainerItem<Instruction, BasicBlock>,
	virtual public User{
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
	void pushToBBL(BasicBlock * bbl);
	void insertSelfAfter(Instruction * pos);
	void insertSelfBefore(Instruction * pos);


private:
	void appendOperands() const {}

	friend class BasicBlock;
};

template <typename Self, Opcode opcode>
class InstructionKind: virtual public Instruction {
protected:
	template <typename Type, char...Name> class ConfigType {
	public:
		ConfigType(Type value) {
			value_ = value;
		}
		ConfigType(const ConfigType& other) {
			value_ = other.value_;
		}
		operator Type() {
			return value_;
		}
	private:
		Type value_;

	};

public:
	typedef ConfigType<size_t, 'B', 'i', 't', 'S', 'i', 'Z', 'e'> BitWidth;
	typedef ConfigType<BasicBlock *, 'B', 'i', 't', 'S', 'i', 'Z', 'e'> EndOfBasicBlock;
	typedef ConfigType<Instruction *, 'A', 'f', 't', 'S', 'i', 'Z', 'e'> AfterInstruction;
	typedef ConfigType<Instruction *, 'A', 'f', 't', 'S', 'i', 'Z', 'e'> BeforeInstruction;

	static Self * dropOwenerShip(std::shared_ptr<Self> self) {
		return self.get();
	}
	static Self * dropOwenerShip(const Self * self) {
		return self;
	}
	bool hasParentDuringCreation_ = false;
	void dropOwnerShip_() {
		hasParentDuringCreation_ = false;
	}

	template<typename ConfigType> static auto config(Self * self, const ConfigType& value) {
		auto pointer = self->template shared_from_this<Self>();
		config(pointer, value);
		return self;
	}
	template<typename ConfigType> static auto config(std::shared_ptr<Self> self, ConfigType value);

	template<> static auto config<BitWidth>(std::shared_ptr<Self> self, BitWidth bitWidth) {
		self->setBitWidth(bitWidth);
		return self;
	}
	template<> static auto config<EndOfBasicBlock>(std::shared_ptr<Self> self, EndOfBasicBlock parent) {
		FATAL_UNLESS(not self->getParent());
		self->pushToBBL(parent);
		return dropOwenerShip(self);
	}
	template<> static auto config<AfterInstruction>(std::shared_ptr<Self> self, AfterInstruction pos) {
		FATAL_UNLESS(not self->getParent());
		self->insertSelfAfter(pos);
		return dropOwenerShip(self);
	}
	template<> static auto config<BeforeInstruction>(std::shared_ptr<Self> self, BeforeInstruction pos) {
		FATAL_UNLESS(not self->getParent());
		self->insertSelfBefore(pos);
		return dropOwenerShip(self);
	}
public:
	template<typename FirstConfig, typename... Args>
	static inline auto create(FirstConfig oneConfig, Args... args) {
		auto retv = create(args...);
		return config(retv, oneConfig);
	}
	template<typename... Args>
	static inline auto create(Value* firstOperand, Args... args) {
		auto retv = create();
		retv->appendOperands(firstOperand, args...);
		return retv;
	}

	/*
	static auto createAfterInstruction(Instruction * pos) {
		return create(AfterInstruction(pos));
	}
	static auto createBeforeInstruction(Instruction * pos) {
		return create(BeforeInstruction(pos));
	}
	static auto create(BasicBlock * bbl) {
		return create(EndOfBasicBlock(bbl));
	}
	static auto create(BasicBlock * parent, size_t bitWidth) {
		return create(EndOfBasicBlock(parent), BitWidth(bitWidth));
	}
	static auto create(size_t bitWidth) {
		return create(BitWidth(bitWidth));
	}
	*/

	static inline auto create() {
		auto retv = std::shared_ptr<Self>(new Self());
		retv->setNumOperands(0);
		return retv;
	}
	virtual Opcode getOpcode() const override final {
		return opcode;
	}
};

class BinaryInstruction: public Instruction {
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
		Value * setLeft(Value * operand) {
			makeSureAtLeastTwoOperands();
			setOperand(0, operand);
		}
		Value * setRight(Value * operand) {
			makeSureAtLeastTwoOperands();
			setOperand(1, operand);
		}
}
class UndefiendInstruction: virtual public InstructionKind<UndefiendInstruction, UNDEF> {};
class NopInstruction: virtual public InstructionKind<NopInstruction, NOP> {};
class AllocInstruction: virtual public InstructionKind<AllocInstruction, ALLOC> {};
