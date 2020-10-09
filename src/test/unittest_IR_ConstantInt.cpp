#include <memory>
#include <gtest/gtest.h>

#include "IR/BasicBlock.hpp"
#include "IR/Instruction.hpp"
#include "IR/Module.hpp"
#include "IR/Constant.hpp"

class IRConstantIntTestSuit: public testing::Test {
protected:
	void SetUp() override {
		module = std::make_shared<Module>();
		a = ConstantInt::create(module.get(), 32, 1);
		b = ConstantInt::create(module.get(), 32, 2);
		c = ConstantInt::create(module.get(), 64, 1);


	}
	void TearDown() override {
		a = nullptr;
		b = nullptr;
		c = nullptr;
		module = nullptr;
	}
	std::shared_ptr<Module> module;
	ConstantInt * a;
	ConstantInt * b;
	ConstantInt * c;
};


TEST_F(IRConstantIntTestSuit, test_stange_bitWidth) {
	try {
		auto d = ConstantInt::create(module.get(), 7, 100);
		FAIL();
	}
	catch (...) {
	}
}

TEST_F(IRConstantIntTestSuit, common_test) {
	ASSERT_EQ(a->getBitWidth(), 32);
	ASSERT_EQ(a->getUnsignedValue(), 1);
}

TEST_F(IRConstantIntTestSuit, same_value) {
	auto d = ConstantInt::create(module.get(), 32, 1);
	ASSERT_EQ(a, d);
}
