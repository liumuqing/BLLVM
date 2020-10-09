#include <gtest/gtest.h>
#include "ModuleLoader/BinaryNinjaModuleLoader.hpp"
#include "IR/Function.hpp"

class BinaryNinjaModuleLoaderTestSuit_SLOW: public testing::Test {
protected:
	static void SetUpTestSuite() {
		binary_module_loader.reset(new BinaryNinjaModuleLoader());
		binary_module_loader->open("./blob/afd.sys");
	}
	static void TearDownTestSuite() {
		//binary_module_loader.release();
	}
	void SetUp() override {
	}
	void TearDown() override {
	}
	static std::unique_ptr<BinaryNinjaModuleLoader> binary_module_loader;
};
std::unique_ptr<BinaryNinjaModuleLoader> BinaryNinjaModuleLoaderTestSuit_SLOW::binary_module_loader = nullptr;

TEST_F(BinaryNinjaModuleLoaderTestSuit_SLOW, getByte) {
	ASSERT_EQ(binary_module_loader->getByteAt(0x1C0084C20), 0x48);
	ASSERT_EQ(binary_module_loader->getByteAt(0x1C0084C21), 0x89);
	ASSERT_EQ(binary_module_loader->getByteAt(0x1C0084C22), 0x5c);
	ASSERT_EQ(binary_module_loader->getByteAt(0x1C0084C24), 0x08);


	//for bad address we should throw an exception
	try {
		binary_module_loader->getByteAt(0x1234);
		FAIL();
	} catch (...) {
	}

	//we are still good to get bytes
	ASSERT_EQ(binary_module_loader->getByteAt(0x1C0084C20), 0x48);
}

TEST_F(BinaryNinjaModuleLoaderTestSuit_SLOW, getSymbolNameAt) {
	ASSERT_STRCASEEQ(binary_module_loader->getSymbolNameAt(0x1C002E068).value_or("").c_str(), "NmrProviderDetachClientComplete@IAT");
}

class BinaryNinjaModuleLoaderTestSuit: public testing::Test {
protected:
	static void SetUpTestSuite() {
		binary_module_loader.reset(new BinaryNinjaModuleLoader());
		binary_module_loader->open("./blob/simple_binary_1/main");
	}
	static void TearDownTestSuite() {
		//binary_module_loader.release();
	}
	void SetUp() override {
	}
	void TearDown() override {
	}
	static std::unique_ptr<BinaryNinjaModuleLoader> binary_module_loader;
};
std::unique_ptr<BinaryNinjaModuleLoader> BinaryNinjaModuleLoaderTestSuit::binary_module_loader = nullptr;

TEST_F(BinaryNinjaModuleLoaderTestSuit, test_lift) {
	auto module = binary_module_loader->lift();
	auto __do_global_dtors_aux = module->getItemByAddress(0x1000);
	ASSERT_NE(__do_global_dtors_aux, nullptr);


}
