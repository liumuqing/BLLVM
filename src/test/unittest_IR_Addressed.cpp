#include <memory>
#include <gtest/gtest.h>
#include "IR/AddressedItem.hpp"
#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"

class IRAddressedTestSuit: public testing::Test {
protected:
	void SetUp() override {
	}
	void TearDown() override {
	}
};

TEST_F(IRAddressedTestSuit, test_bbl_constructor) {
	auto bbl1 = BasicBlock::create(0x1234);
	ASSERT_EQ(bbl1->getAddress(), 0x1234);
}
TEST_F(IRAddressedTestSuit, test_1) {
	auto func1 = std::make_shared<Function>();

	auto bbl3 = BasicBlock::create();
	auto bbl2 = BasicBlock::create(0x2000);
	func1->addAddressedItem(bbl2);
	auto bbl1 = BasicBlock::create(func1, 0x1000);

	func1->addAddressedItem(0x3000, bbl3);

	int index = 0;
	for (auto bbl : func1->children()) {
		EXPECT_EQ(bbl->getAddress(), (index+1) * 0x1000);
		index += 1;
	}

	EXPECT_EQ(index, 3);


}
