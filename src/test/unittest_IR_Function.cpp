#include <memory>
#include <gtest/gtest.h>
#include "IR/Module.hpp"
#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"

class IRFunctionTestSuit: public testing::Test {
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

TEST_F(IRFunctionTestSuit, test_function_paramter_index) {
    Parameter * a = function->createAndAppendParameter(16);
    Parameter * b = function->createAndAppendParameter(32);
    Parameter * c = function->createAndAppendParameter(64);
    Parameter * d = function->createAndAppendParameter(32);

    EXPECT_EQ(function->countOfParamter(), 4);

    EXPECT_EQ(a->getIndex(), 0);
    EXPECT_EQ(b->getIndex(), 1);
    EXPECT_EQ(c->getIndex(), 2);
    EXPECT_EQ(d->getIndex(), 3);

    EXPECT_EQ(a->getBitWidth(), 16);
    EXPECT_EQ(b->getBitWidth(), 32);
    EXPECT_EQ(c->getBitWidth(), 64);
    EXPECT_EQ(d->getBitWidth(), 32);
}
