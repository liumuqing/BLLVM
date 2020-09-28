#include <memory>
#include <gtest/gtest.h>
#include "IR/AddressedItem.hpp"
#include "IR/Module.hpp"
#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"

class IRAddressedTestSuit: public testing::Test {
protected:
	void SetUp() override {
		module = std::make_shared<Module>();
		function = Function::create(module, 0x1000);
	}
	void TearDown() override {
	}
	std::shared_ptr<Module> module;
	std::shared_ptr<Function> function;
};

TEST_F(IRAddressedTestSuit, test_bbl_constructor) {
	auto bbl1 = BasicBlock::create(function, 0x1234);
	ASSERT_EQ(bbl1->getAddress(), 0x1234);
}
TEST_F(IRAddressedTestSuit, test_1) {

	auto bbl3 = BasicBlock::create();
	auto bbl2 = BasicBlock::create(function, 0x2000);
	auto bbl1 = BasicBlock::create(function, 0x1000);
	bbl3->setAddress(0x3000);
	function->push_back(bbl3);

	int index = 0;
	for (auto bbl : *function) {
		index += 1;
	}

	EXPECT_EQ(index, 3);


}
