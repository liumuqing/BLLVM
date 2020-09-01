#include "IR/User.hpp"
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
	ZX,
	SX,
	LOAD,
	STORE,
	CMP_EQ,
	CMP_LT,
	CALL,
	CONST_INT,
	//BBL Terminator
	SWITCH,
	BRANCH,
	//Anything, float instruction
	UNDEF,
	//Function Terminator
	RETURN,
	UNREACHABLE,
};

class Instruction: public User {
public:

};

class BinaryOperator: public Instruction {

}
