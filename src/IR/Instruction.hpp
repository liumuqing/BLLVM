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


private:
	void appendOperands() const {}

	friend class BasicBlock;
};

template <typename Self, Opcode opcode>
class InstructionKind: virtual public Instruction{
public:
	static Self* create(BasicBlock * bbl) {
		auto retv = create();
		retv->pushToBBL(bbl);
		return retv.get();
	}

	template<typename... Args>
	static inline Self* create(BasicBlock * bbl, Args... args) {
		auto retv = create(bbl);
		retv->appendOperands(args...);
		return retv;
	}
	template<typename... Args>
	static inline std::shared_ptr<Instruction> create(Args... args) {
		auto retv = create();
		retv->appendOperands(args...);
		return retv;
	}
	static inline std::shared_ptr<Self> create() {
		auto retv = std::shared_ptr<Self>(new Self());
		retv->setNumOperands(0);
		return retv;
	}
	virtual Opcode getOpcode() const override final {
		return opcode;
	}
};

class UndefiendInstruction: virtual public InstructionKind<UndefiendInstruction, UNDEF> {};
class NopInstruction: virtual public InstructionKind<NopInstruction, NOP> {};
class AllocInstruction: virtual public InstructionKind<AllocInstruction, ALLOC> {};
