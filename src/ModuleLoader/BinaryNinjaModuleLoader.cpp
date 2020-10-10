#include "ModuleLoader/BinaryNinjaModuleLoader.hpp"
#include <mediumlevelilinstruction.h>

#include <binaryninjacore.h>
#include <binaryninjaapi.h>

#include <string>
#include <filesystem>

#include <plog/Log.h>

#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Instruction.hpp"
#include "IR/Constant.hpp"
#include "common.hpp"

struct LiftFunctionContext {
	BinaryNinja::Ref<BinaryNinja::MediumLevelILFunction> ssa_form = nullptr;
	BasicBlock * entryBasicBlock = nullptr;
	std::shared_ptr<Function> function = nullptr;

	std::map<BinaryNinja::ExprId, Instruction *> exprId2Instruction;
	std::map<size_t, Instruction *> instId2Instruction;
	std::map<BinaryNinja::Ref<BinaryNinja::BasicBlock>, BasicBlock *> bnBB2BB;

	std::map<BinaryNinja::SSAVariable, std::optional<std::shared_ptr<Value>>> SSAVariableToInst;
	std::map<BinaryNinja::Variable, std::shared_ptr<Instruction>> memoryVariableToAllocInst;
};

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
static size_t getBitWidthOfVariable(LiftFunctionContext& ctx, const BinaryNinja::Variable& var) {
	auto paramaterType = ctx.ssa_form->GetFunction()->GetVariableType(var);
	if (not paramaterType) {
		FATAL("BinaryNinja cannot infer parameter's type...");
	}
	return paramaterType->GetWidth() * 8;
}

static size_t getBitWidthOfSSAVariable(LiftFunctionContext& ctx, const BinaryNinja::SSAVariable& ssaVar) {
	return getBitWidthOfVariable(ctx, ssaVar.var);
}

static void lift_function_step_1_create_function_parameters(LiftFunctionContext& ctx) {
	{
		auto parameter_vars = ctx.ssa_form->GetFunction()->GetParameterVariables().GetValue();
		for (size_t index = 0, count = parameter_vars.size(); index < count; index ++) {
			//check paramaterType->GetClass(), see if it's bool type...
			auto para = ctx.function->createAndAppendParameter(
					getBitWidthOfVariable(ctx, parameter_vars[index])
					);
			ctx.SSAVariableToInst[BinaryNinja::SSAVariable(parameter_vars[index], 0)] = para->shared_from_this();
		}
	}
}

static void lift_function_step_2_create_dummy_inst(LiftFunctionContext& ctx) {
	for (size_t expr_id = 0, count = ctx.ssa_form->GetExprCount(); expr_id< count; expr_id++) {
		auto expr = ctx.ssa_form->GetExpr(expr_id);
		auto inst_id = ctx.ssa_form->GetInstructionForExpr(expr_id);
		auto ssa_inst = ctx.ssa_form->GetInstruction(inst_id);
		auto ssa_bbl = ctx.ssa_form->GetBasicBlockForInstruction(inst_id);

		if (!ctx.bnBB2BB.contains(ssa_bbl)) {
			ctx.bnBB2BB[ssa_bbl] = BasicBlock::create(ctx.function.get());
		}
		auto bbl = ctx.bnBB2BB[ssa_bbl];
		auto inst = NopInstruction::create(bbl);
		ctx.exprId2Instruction[expr_id] = inst;
		if (ctx.ssa_form->GetIndexForInstruction(inst_id) == expr_id) {
			ctx.instId2Instruction[inst_id] = inst;
		}
	}
}

auto isMemorySSA(LiftFunctionContext& ctx, const BinaryNinja::SSAVariable& ssaVar) {
	size_t ssa_def_inst_id = ctx.ssa_form->GetSSAVarDefinition(ssaVar);
	size_t memory_def_inst_id = ctx.ssa_form->GetSSAMemoryDefinition(ssaVar.version);
	size_t inst_count = ctx.ssa_form->GetInstructionCount();
	if (ssa_def_inst_id < inst_count and memory_def_inst_id < inst_count and ssa_def_inst_id == memory_def_inst_id) {
		auto ssa_inst = ctx.ssa_form->GetInstruction(ssa_def_inst_id);
		std::set<BNMediumLevelILOperation> black_ops = {
			BNMediumLevelILOperation::MLIL_CALL_SSA,
			BNMediumLevelILOperation::MLIL_SYSCALL_SSA,
		};
		if (black_ops.contains(ssa_inst.operation)) {
			return false;
		}
		return true;
	}
	return false;
};

//return the dummy instruction
auto getSSAVariableValue(LiftFunctionContext& ctx, const BinaryNinja::SSAVariable& ssaVar) {
	FATAL_UNLESS(not isMemorySSA(ctx, ssaVar));
	auto iter = ctx.SSAVariableToInst.find(ssaVar);

	if (iter == ctx.SSAVariableToInst.end()){

		// we don't need care of if ssaVar is a parameter or not..
		// if ssaVar is a parameter, it will found in this map.
		// then if there is no definition for this ssaVar, we just mark it as undefined, and it should be in entryBBL
		auto inst_id = ctx.ssa_form->GetSSAVarDefinition(ssaVar);
		if (inst_id >= ctx.ssa_form->GetInstructionCount()) {
			auto newInst = UndefiendInstruction::create();
			ctx.entryBasicBlock->insertInstructionAtBegin(newInst);

			auto [newIter, isInserted] = ctx.SSAVariableToInst.insert(std::make_pair(ssaVar, newInst));
			FATAL_UNLESS(isInserted);
			iter = newIter;

		} else {
			auto newInst = UndefiendInstruction::create();
			FATAL_UNLESS(ctx.instId2Instruction.contains(inst_id));
			auto insertPos = ctx.instId2Instruction[inst_id];
			insertPos->getParent()->insertInstructionAfter(insertPos, newInst);

			auto [newIter, isInserted] = ctx.SSAVariableToInst.insert(std::make_pair(ssaVar, newInst));
			FATAL_UNLESS(isInserted);
			iter = newIter;
		}
	}

	FATAL_UNLESS(iter != ctx.SSAVariableToInst.end());
	return iter->second;
}

auto getMemoryVariableAllocInst(LiftFunctionContext& ctx, const BinaryNinja::SSAVariable& ssaVar) {
	FATAL_UNLESS(isMemorySSA(ctx, ssaVar));
	auto iter = ctx.memoryVariableToAllocInst.find(ssaVar.var);
	if (iter == ctx.memoryVariableToAllocInst.end()){
		size_t pointerSizeInBits = ctx.ssa_form->GetFunction()->GetPlatform()->GetArchitecture()->GetAddressSize() * 8;
		FATAL_UNLESS(pointerSizeInBits == 16 || pointerSizeInBits == 32 || pointerSizeInBits == 64);

		auto new_inst = AllocInstruction::create();
		new_inst->appendOperands(ConstantInt::create(ctx.function->getParent(), pointerSizeInBits, getBitWidthOfSSAVariable(ctx, ssaVar)));
		new_inst->setBitWidth(pointerSizeInBits);
		ctx.entryBasicBlock->insertInstructionAtBegin(new_inst);

		auto [newIter, isInserted] = ctx.memoryVariableToAllocInst.insert(std::make_pair(ssaVar.var, new_inst));
		FATAL_UNLESS(isInserted);

		iter = newIter;
	}

	FATAL_UNLESS(iter != ctx.memoryVariableToAllocInst.end());
	return iter->second;
}

Function* lift_function(Module * module, BinaryNinja::Ref<BinaryNinja::MediumLevelILFunction> ssa_form) {

	FATAL_UNLESS(ssa_form->GetInstructionCount() > 0);

	LiftFunctionContext ctx;
	ctx.ssa_form = ssa_form;
	ctx.function = Function::create(ssa_form->GetFunction()->GetStart());


	//Create Function Parameters
	lift_function_step_1_create_function_parameters(ctx);

	//Create Dummy Instruction And BBL
	lift_function_step_2_create_dummy_inst(ctx);

	//Set entry bbl in ctx
	auto entryBNBBL = ctx.ssa_form->GetBasicBlockForInstruction(0);
	FATAL_UNLESS(entryBNBBL);
	ctx.entryBasicBlock = ctx.bnBB2BB[entryBNBBL];




	//Collect all

	//Now we have all dummy instructions and dummy basicblocks created, in `exprId2Instruction` and `bnbbl2bbl`
	//Let's translate them....
	//



	for (auto &&[expr_id, my_inst]: ctx.exprId2Instruction) {
		//translate each expr to Instruction
		auto expr = ssa_form->GetExpr(expr_id);
		switch (expr.operation) {
			case BNMediumLevelILOperation::MLIL_NOP:
				break;
			case BNMediumLevelILOperation::MLIL_ADD:
				//TODO
				my_inst->setBitWidth(expr.size * 8);
				break;


		}
	}


	/*
	for (auto ssa_bbl: ssa_form->GetBasicBlocks()) {
		if (not liftBasicBlock(exprId2Instruction, function.get(), ssa_bbl)) {
			return nullptr;
		}
	}
	*/
	module->push_back(ctx.function);
	return ctx.function.get();
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
