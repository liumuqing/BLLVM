#include <memory>
#include <gtest/gtest.h>
#include "IR/Use.hpp"
#include "IR/Value.hpp"
#include "IR/User.hpp"

class IRUseTestSuit: public testing::Test {
protected:
    void SetUp() override {
        user_12 = std::make_shared<User>();
        user_345 = std::make_shared<User>();

        value1 = std::make_shared<Value>();
        value2 = std::make_shared<Value>();
        value3 = std::make_shared<Value>();
        value4 = std::make_shared<Value>();
        value5 = std::make_shared<Value>();

        user_12->setNumOperands(2);
        user_12->setOperand(0, value1.get());
        user_12->setOperand(1, value2.get());

        user_345->setNumOperands(3);
        user_345->setOperand(0, value3.get());
        user_345->setOperand(1, value4.get());
        user_345->setOperand(2, value5.get());
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
    std::shared_ptr<User> user_12;
    std::shared_ptr<Value> value1;
    std::shared_ptr<Value> value2;

    std::shared_ptr<User> user_345;
    std::shared_ptr<Value> value3;
    std::shared_ptr<Value> value4;
    std::shared_ptr<Value> value5;
};


TEST_F(IRUseTestSuit, test_1) {
    user_12->replaceUsesOfWith(value2.get(), value1.get());

    EXPECT_EQ(user_12->getOperand(0), value1.get());
    EXPECT_EQ(user_12->getOperand(1), value1.get());
}

TEST_F(IRUseTestSuit, test_2) {
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

TEST_F(IRUseTestSuit, test_3) {
    user_345->setOperand(0, value2.get());
    value2->replaceUsesWith(value1.get());

    EXPECT_EQ(user_12->getOperand(0), value1.get());
    EXPECT_EQ(user_12->getOperand(1), value1.get());

    EXPECT_EQ(user_345->getOperand(0), value1.get());
    EXPECT_EQ(user_345->getOperand(1), value4.get());
    EXPECT_EQ(user_345->getOperand(2), value5.get());
}
