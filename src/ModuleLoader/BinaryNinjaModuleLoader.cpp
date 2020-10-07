#include "ModuleLoader/BinaryNinjaModuleLoader.hpp"
#include <mediumlevelilinstruction.h>

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
		//after we uninit all resource, we are able shutdown BN
		BNShutdown();
	}
}

void BinaryNinjaModuleLoader::open(const char *path) {
	FATAL_UNLESS (not isOpen());

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
	FATAL_UNLESS(isOpen());
	return binary_view->GetDefaultEndianness() == BigEndian;
}

uint8_t BinaryNinjaModuleLoader::getByteAt(uaddr_t addr) {
	FATAL_UNLESS(isOpen());
	binary_reader->Seek(addr);

	uint8_t retv;
	if (not binary_reader->TryRead8(retv)) {
		FATAL("cannot read at %p", addr);
	}
	return retv;
}

std::optional<std::string> BinaryNinjaModuleLoader::getSymbolNameAt(uaddr_t addr) {
	FATAL_UNLESS(isOpen());
	BinaryNinja::Ref<BinaryNinja::Symbol> symbol = binary_view->GetSymbolByAddress(addr);

	if (!symbol) return {};
	return symbol->GetFullName();
}

bool liftInstruction(std::map<BinaryNinja::ExprId, Instruction *>& exprId2Instruction, BasicBlock * basicblock, BinaryNinja::Ref<BinaryNinja::MediumLevelILInstruction> ssa_inst) {
	return true;
}
BasicBlock* liftBasicBlock(std::map<BinaryNinja::ExprId, Instruction *>& exprId2Instruction, Function * function, BinaryNinja::Ref<BinaryNinja::BasicBlock> ssa_bbl) {
	auto bbl = BasicBlock::create(ssa_bbl->GetStart());
	function->push_back(bbl);
	return bbl.get();
}

Function* lift_function(Module * module, BinaryNinja::Ref<BinaryNinja::MediumLevelILFunction> ssa_form) {
	std::map<BinaryNinja::ExprId, Instruction *> exprId2Instruction;
	std::map<BinaryNinja::Ref<BinaryNinja::BasicBlock>, BasicBlock *> bnbbl2bbl;
	auto function = Function::create(ssa_form->GetFunction()->GetStart());
	for (size_t expr_id = 0, count = ssa_form->GetExprCount(); expr_id< count; expr_id++) {
		auto expr = ssa_form->GetExpr(expr_id);
		auto inst_id = ssa_form->GetInstructionForExpr(expr_id);
		auto ssa_bbl = ssa_form->GetBasicBlockForInstruction(inst_id);

		if (!bnbbl2bbl.contains(ssa_bbl)) {
			bnbbl2bbl[ssa_bbl] = BasicBlock::create(function.get()).get();
		}
		auto bbl = bnbbl2bbl[ssa_bbl];
		auto inst = Instruction::create(bbl);
		exprId2Instruction[expr_id] = inst.get();
	}

	//Now we have all dummy instructions and dummy basicblocks created, in `exprId2Instruction` and `bnbbl2bbl`
	//Let's translate them....
	//TODO:

	//for ([&expr_id, &my_inst]: exprId2Instruction) {
	//}



	for (auto ssa_bbl: ssa_form->GetBasicBlocks()) {
		if (not liftBasicBlock(exprId2Instruction, function.get(), ssa_bbl)) {
			return nullptr;
		}
	}
	module->push_back(function);
	return function.get();
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
