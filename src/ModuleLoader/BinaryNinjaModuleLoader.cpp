#include "ModuleLoader/BinaryNinjaModuleLoader.hpp"

#include <binaryninjaapi.h>

#include <string>
#include <filesystem>

#include <plog/Log.h>

#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"

std::atomic<int> BinaryNinjaModuleLoader::instance_count;

BinaryNinjaModuleLoader::BinaryNinjaModuleLoader() {
	binary_reader = nullptr;
	binary_view = nullptr;
	binary_data_view = nullptr;

	if (instance_count.fetch_add(1) == 0) {
		//If this is the first instance
		BinaryNinja::SetBundledPluginDirectory("/opt/binaryninja/plugins/");
		BinaryNinja::InitCorePlugins();
		BinaryNinja::InitUserPlugins();
	}
};

BinaryNinjaModuleLoader::~BinaryNinjaModuleLoader() {
	binary_reader = nullptr;
	binary_view = nullptr;
	binary_data_view = nullptr;

	if (instance_count.fetch_sub(1) == 1) {
		//If this is the last instance
		//after we deinit all resource, we are able shutdown BN
		BNShutdown();
	}
}

void BinaryNinjaModuleLoader::open(const char *path) {
	assert (not isOpen());

	std::string input_path = path;
	std::string bndb_path = (input_path.ends_with(".bndb") ? input_path : input_path + ".bndb");

	if (not std::filesystem::exists(bndb_path)) {
		binary_data_view = new BinaryNinja::BinaryData(new BinaryNinja::FileMetadata(), input_path);
	} else {
		binary_data_view = BinaryNinja::FileMetadata().OpenExistingDatabase(bndb_path);
	}

	for (auto type : BinaryNinja::BinaryViewType::GetViewTypes())
	{
		if (type->IsTypeValidForData(binary_data_view) && type->GetName() != "Raw")
		{
			binary_view = type->Create(binary_data_view);
			break;
		}
	}

	if (!binary_view || binary_view->GetTypeName() == "Raw")
	{
		LOGF << "Input file does not appear to be an exectuable\n";
		exit(1);
	}

	LOG_DEBUG << "use BinaryViewType " << binary_view->GetTypeName();

	LOG_INFO << "binary file loaded, wait analysis...";
	binary_view->UpdateAnalysisAndWait();
	INFO("binary file loaded, analysis finish");

	if (not std::filesystem::exists(bndb_path)) {
		binary_view->CreateDatabase(input_path+".bndb");
		INFO("database saved");
	}
	binary_reader.reset(new BinaryNinja::BinaryReader(binary_view));
}

bool BinaryNinjaModuleLoader::isBigEndian() {
	assert(isOpen());
	return binary_view->GetDefaultEndianness() == BigEndian;
}

uint8_t BinaryNinjaModuleLoader::getByteAt(uaddr_t addr) {
	assert(isOpen());
	binary_reader->Seek(addr);

	uint8_t retv;
	if (not binary_reader->TryRead8(retv)) {
		FATAL("cannot read at %p", addr);
	}
	return retv;
}

std::optional<std::string> BinaryNinjaModuleLoader::getSymbolNameAt(uaddr_t addr) {
	BinaryNinja::Ref<BinaryNinja::Symbol> symbol = binary_view->GetSymbolByAddress(addr);
	if (!symbol) return {};
	return symbol->GetFullName();
}

bool lift_function(Module * module, BinaryNinja::Ref<BinaryNinja::MediumLevelILFunction> ssa_form) {
	auto function = Function::create(module, ssa_form->GetFunction()->GetStart());

	for (auto ssa_bbl: ssa_form->GetBasicBlocks()) {
		auto bbl = BasicBlock::create(function, ssa_bbl->GetStart());
	}

	return true;
}

std::shared_ptr<Module> BinaryNinjaModuleLoader::lift() {
	std::shared_ptr<Module> module = std::make_shared<Module>();
	for (auto function: binary_view->GetAnalysisFunctionList()) {
		auto mlil = function->GetMediumLevelILIfAvailable();
		if (!mlil) {
			LOG_INFO << "lifting function " << std::hex << function->GetStart() << "failed, skip";
			continue;
		}
		auto ssa_form = mlil->GetSSAForm();
		if (!ssa_form) {
			LOG_INFO << "lifting function " << std::hex << function->GetStart() << " to ssa_form failed, skip";
			continue;
		}

		lift_function(module.get(), ssa_form);

		LOG_DEBUG << "lifting function " << std::hex << function->GetStart();
	}
	return module;
}
