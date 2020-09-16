#include <memory>
#include <gtest/gtest.h>
#include "IR/Instruction.hpp"

class IRInstructionTestSuit: public testing::Test {
protected:
	void SetUp() override {

		value1 = Instruction::create(NOP);
		value2 = Instruction::create(NOP);
		value3 = Instruction::create(NOP);
		value4 = Instruction::create(NOP);
		value5 = Instruction::create(NOP);

		user_12 = Instruction::create(NOP, value1.get(), value2.get());
		user_345 = Instruction::create(NOP, value3.get(), value4.get(), value5.get());
	}
	void TearDown() override {
		user_12 = nullptr;
		user_345 = nullptr;
		value1 = nullptr;
		value2 = nullptr;
		value3 = nullptr;
		value4 = nullptr;
		value5 = nullptr;
	}
	std::shared_ptr<Instruction> user_12;
	std::shared_ptr<Instruction> value1;
	std::shared_ptr<Instruction> value2;

	std::shared_ptr<Instruction> user_345;
	std::shared_ptr<Instruction> value3;
	std::shared_ptr<Instruction> value4;
	std::shared_ptr<Instruction> value5;
};


TEST_F(IRInstructionTestSuit, test_1) {
	user_12->replaceUsesOfWith(value2.get(), value1.get());

	EXPECT_EQ(user_12->getOperand(0), value1.get());
	EXPECT_EQ(user_12->getOperand(1), value1.get());
}

TEST_F(IRInstructionTestSuit, test_2) {
	user_345->setOperand(0, value2.get());
	user_12->replaceUsesOfWith(value2.get(), value1.get());

	EXPECT_EQ(user_12->getOperand(0), value1.get());
	EXPECT_EQ(user_12->getOperand(1), value1.get());

	EXPECT_EQ(user_345->getOperand(0), value2.get());
	EXPECT_EQ(user_345->getOperand(1), value4.get());
	EXPECT_EQ(user_345->getOperand(2), value5.get());

	user_345->replaceUsesOfWith(value2.get(), value1.get());
	EXPECT_EQ(user_345->getOperand(0), value1.get());

}
