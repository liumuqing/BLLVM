#include <memory>
#include <atomic>
#include <mutex>

#include "binaryninjaapi.h"
#include "ModuleLoader/ModuleLoader.hpp"
class BinaryNinjaModuleLoader: ModuleLoader {

public:
	BinaryNinjaModuleLoader();
	virtual void open(const char *) override;
	virtual std::shared_ptr<Module> lift() override;
	virtual uint8_t getByteAt(uaddr_t addr) override;
	virtual bool isBigEndian() override;
	virtual std::optional<std::string> getSymbolNameAt(uaddr_t addr) override;
	virtual ~BinaryNinjaModuleLoader() override;
private:
	static std::atomic<int> instance_count;

	//We must use Ref, instead of std::shared_ptr

	BinaryNinja::Ref<BinaryNinja::BinaryView> binary_data_view;
	BinaryNinja::Ref<BinaryNinja::BinaryView> binary_view;
	std::unique_ptr<BinaryNinja::BinaryReader> binary_reader;
protected:
	bool isOpen() {
		return binary_view;
	}
};
