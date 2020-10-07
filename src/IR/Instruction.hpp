#pragma once
#include "IR/AddressedItem.hpp"
#include "IR/User.hpp"
#include "IR/BasicBlock.hpp"
enum Opcode {
	NOP,
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

class Instruction: virtual public User, virtual public ListContainerItem<Instruction, BasicBlock> {
public:
	static inline std::shared_ptr<Instruction> create(BasicBlock * bbl) {
		return create(bbl, NOP);
	}
	static std::shared_ptr<Instruction> create(BasicBlock * bbl, Opcode opcode);

	template<typename... Args>
	static inline std::shared_ptr<Instruction> create(BasicBlock * bbl, Opcode opcode, Args... args) {
		auto retv = create(bbl, opcode);
		retv->appendOperands(args...);
		return retv;
	}
	static inline std::shared_ptr<Instruction> create(Opcode opcode) {
		auto retv = std::shared_ptr<Instruction>(new Instruction());
		retv->opcode_ = opcode;
		retv->setNumOperands(0);
		return retv;
	}
	template<typename... Args>
	static inline std::shared_ptr<Instruction> create(Opcode opcode, Args... args) {
		auto retv = create(opcode);
		retv->appendOperands(args...);
		return retv;
	}
	template<typename... Args>
	void appendOperands(Instruction * firstOperand, Args... remainingOperands) {
		this->setNumOperands(this->getNumOperands()+1);
		this->setOperand(this->getNumOperands()-1, firstOperand);
		if (sizeof...(remainingOperands)) {
			this->appendOperands(remainingOperands...);
		}
	}

	void setOpcode(Opcode opcode);
	Opcode getOpcode() const;
protected:
	Instruction():opcode_(UNDEF){};
private:
	Opcode opcode_;
	void appendOperands() const {}

	friend class BasicBlock;
};

class BinaryOp: virtual public Instruction {

};
