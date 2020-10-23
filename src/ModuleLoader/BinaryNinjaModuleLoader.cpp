#include "ModuleLoader/BinaryNinjaModuleLoader.hpp"
#include <mediumlevelilinstruction.h>

#include <binaryninjacore.h>
#include <binaryninjaapi.h>

#include <string>
#include <filesystem>
#include <variant>

#include <plog/Log.h>

#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Instruction.hpp"
#include "IR/Constant.hpp"
#include "common.hpp"

struct LiftFunctionContext {
    BinaryNinja::Ref<BinaryNinja::MediumLevelILFunction> ssa_form = nullptr;
    BasicBlock * entryBasicBlock = nullptr;
    std::weak_ptr<Module> module;
    std::shared_ptr<Function> function = nullptr;

    std::map<BinaryNinja::ExprId, Instruction *> exprId2Instruction;
    std::map<size_t, Instruction *> instId2Instruction;
    std::map<BinaryNinja::Ref<BinaryNinja::BasicBlock>, BasicBlock *> bnBB2BB;

    std::map<BinaryNinja::SSAVariable, std::shared_ptr<Value>> SSAVariableToInst;
    std::map<BinaryNinja::Variable, std::shared_ptr<AllocInstruction>> memoryVariableToAllocInst;
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
        WARN("BinaryNinja cannot infer variable's type..., return as 0");
        return 0;
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
        auto inst = UndefiendInstruction::create(UndefiendInstruction::EndOfBasicBlock(bbl));
        ctx.exprId2Instruction[expr_id] = inst;
        if (ctx.ssa_form->GetIndexForInstruction(inst_id) == expr_id) {
            ctx.instId2Instruction[inst_id] = inst;
        }
    }
}

auto static isMemorySSA(LiftFunctionContext& ctx, const BinaryNinja::SSAVariable& ssaVar) {
    //FIXME: this function is wrong
    // but let's just use it, until I'm ready to fix it
    size_t ssa_def_inst_id = ctx.ssa_form->GetSSAVarDefinition(ssaVar);
    size_t memory_def_inst_id = ctx.ssa_form->GetSSAMemoryDefinition(ssaVar.version);
    size_t inst_count = ctx.ssa_form->GetInstructionCount();
    if (ssa_def_inst_id < inst_count and memory_def_inst_id < inst_count and ssa_def_inst_id == memory_def_inst_id) {
        auto ssa_inst = ctx.ssa_form->GetInstruction(ssa_def_inst_id);
        std::set<BNMediumLevelILOperation> black_ops = {
            BNMediumLevelILOperation::MLIL_CALL_SSA,
            BNMediumLevelILOperation::MLIL_CALL_UNTYPED_SSA,
            BNMediumLevelILOperation::MLIL_SYSCALL_SSA,
            BNMediumLevelILOperation::MLIL_TAILCALL_SSA,
            BNMediumLevelILOperation::MLIL_TAILCALL_UNTYPED_SSA,
            //FIXME
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
            auto parent = dynamic_cast<BasicBlock *>(insertPos->getParent());
            FATAL_UNLESS(parent);
            parent->insertInstructionAfter(insertPos, newInst);

            auto [newIter, isInserted] = ctx.SSAVariableToInst.insert(std::make_pair(ssaVar, newInst));
            FATAL_UNLESS(isInserted);
            iter = newIter;
        }
    }

    FATAL_UNLESS(iter != ctx.SSAVariableToInst.end());
    return iter->second.get();
}

AllocInstruction *getMemoryVariableAllocInst(LiftFunctionContext& ctx, const BinaryNinja::Variable& var) {
    auto iter = ctx.memoryVariableToAllocInst.find(var);
    if (iter == ctx.memoryVariableToAllocInst.end()){
        size_t pointerSizeInBits = ctx.ssa_form->GetFunction()->GetPlatform()->GetArchitecture()->GetAddressSize() * 8;
        FATAL_UNLESS(pointerSizeInBits == 16 || pointerSizeInBits == 32 || pointerSizeInBits == 64);

        auto new_inst = AllocInstruction::create(
                AllocInstruction::BitWidth(pointerSizeInBits),
                AllocInstruction::VariableBitWidth(getBitWidthOfVariable(ctx, var))
                );
        //new_inst->setAllocatedBitSize(getBitWidthOfSSAVariable(ctx, ssaVar));
        //new_inst->setBitWidth(pointerSizeInBits);
        ctx.entryBasicBlock->insertInstructionAtBegin(new_inst);

        auto [newIter, isInserted] = ctx.memoryVariableToAllocInst.insert(std::make_pair(var, new_inst));
        FATAL_UNLESS(isInserted);

        iter = newIter;
    }

    FATAL_UNLESS(iter != ctx.memoryVariableToAllocInst.end());
    return iter->second.get();
}

AllocInstruction *getMemorySSAVariableAllocInst(LiftFunctionContext& ctx, const BinaryNinja::SSAVariable& ssaVar) {
    FATAL_UNLESS(isMemorySSA(ctx, ssaVar));
    return getMemoryVariableAllocInst(ctx, ssaVar.var);
}

Function* lift_function(Module * module, BinaryNinja::Ref<BinaryNinja::MediumLevelILFunction> ssa_form) {

    FATAL_UNLESS(ssa_form->GetInstructionCount() > 0);

    LiftFunctionContext ctx;
    ctx.module = module->weak_from_this<Module>();
    ctx.ssa_form = ssa_form;
    ctx.function = Function::create(ssa_form->GetFunction()->GetStart());


    //Create Function Parameters
    lift_function_step_1_create_function_parameters(ctx);

    //Create Dummy Instruction And BBL
    lift_function_step_2_create_dummy_inst(ctx);

    //Set entry bbl in ctx
    auto entryBNBBL = ctx.ssa_form->GetBasicBlockForInstruction(0);
    FATAL_UNLESS(entryBNBBL);
    FATAL_UNLESS(ctx.bnBB2BB.contains(entryBNBBL));
    ctx.entryBasicBlock = ctx.bnBB2BB[entryBNBBL];




    //Collect all

    //Now we have all dummy instructions and dummy basicblocks created, in `exprId2Instruction` and `bnbbl2bbl`
    //Let's translate them....
    //



    for (auto [expr_id, placeholderInst]: ctx.exprId2Instruction) {
        //translate each expr to Instruction
        auto expr = ssa_form->GetExpr(expr_id);
        Instruction * newInst = nullptr;
        auto getTranslatedReadOperand = [&ctx](
                std::variant<const BinaryNinja::SSAVariable, const BinaryNinja::MediumLevelILInstruction> operand,
                Instruction * placeholderInst) -> Value *{
            if (operand.index() == 0) {
                auto ssaVar = std::get<0>(operand);
                if (isMemorySSA(ctx, ssaVar)) {
                    auto allocInst = getMemorySSAVariableAllocInst(ctx, ssaVar);
                    auto loadInst = LoadInstruction::create(
                            LoadInstruction::BeforeInstruction(placeholderInst),
                            LoadInstruction::BitWidth(allocInst->getVariableBitWidth()),
                            allocInst
                            );
                    return loadInst;
                } else {
                    return getSSAVariableValue(ctx, ssaVar);
                }
            } else {
                auto expr = std::get<1>(operand);
                auto expr_id = expr.exprIndex;
                auto iter = ctx.exprId2Instruction.find(expr_id);
                FATAL_UNLESS(iter != ctx.exprId2Instruction.end());
                return iter->second;
            }
        };
        auto replaceWith = [&ctx](BinaryNinja::ExprId expr_id, Instruction * value) {
            FATAL_UNLESS(ctx.exprId2Instruction.contains(expr_id));

            auto expr = ctx.ssa_form->GetExpr(expr_id);
            auto inst_id = ctx.ssa_form->GetInstructionForExpr(expr_id);
            if (ctx.ssa_form->GetIndexForInstruction(inst_id) == expr_id) {
                ctx.instId2Instruction[inst_id] = value;
            }

            auto old_value = ctx.exprId2Instruction[expr_id];
            old_value->replaceUsesWith(value);
            ctx.exprId2Instruction[expr_id] = value;
        };
        auto writeSSAvariable = [&ctx](BinaryNinja::SSAVariable ssaVar, Instruction * value) -> void {
            if (isMemorySSA(ctx, ssaVar)) {
                auto store_inst = StoreInstruction::create(
                        StoreInstruction::BitWidth(getBitWidthOfSSAVariable(ctx, ssaVar)),
                        StoreInstruction::AfterInstruction(value),
                        getMemorySSAVariableAllocInst(ctx, ssaVar),
                        value
                        );
                return;
            }
            else {
                //return;
                auto old_value = dynamic_cast<Instruction *>(getSSAVariableValue(ctx, ssaVar));
                auto new_inst = MovInstruction::create(
                        MovInstruction::AfterInstruction(value),
                        MovInstruction::BitWidth(value->getBitWidth()),
                        value
                        );

                old_value->replaceUsesWith(new_inst);
                ctx.SSAVariableToInst[ssaVar] = new_inst->shared_from_this();
                old_value->removeFromParent();
            }
        };
        switch (expr.operation) {
            #define DEFINE_OPCODE_INSTRUCTION(bn_opcode, InstructionType) \
            case BNMediumLevelILOperation::bn_opcode: { \
                    newInst = InstructionType::create( \
                        InstructionType::AfterInstruction(placeholderInst),\
                        InstructionType::BitWidth(expr.size * 8)\
                        );\
                    break;\
                }
            DEFINE_OPCODE_INSTRUCTION(MLIL_NOP, NopInstruction);

            #define DEFINE_BINARY_INSTRUCTION(bn_opcode, InstructionType) \
            case BNMediumLevelILOperation::bn_opcode: { \
                    newInst = InstructionType::create( \
                        InstructionType::AfterInstruction(placeholderInst),\
                        InstructionType::BitWidth(expr.size * 8),\
                        getTranslatedReadOperand(expr.GetLeftExpr(), placeholderInst),\
                        getTranslatedReadOperand(expr.GetRightExpr(), placeholderInst)\
                        );\
                    break;\
                }
            DEFINE_BINARY_INSTRUCTION(MLIL_SUB, SubInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_ADD, AddInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_ADD_OVERFLOW, AddInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_MUL, MulInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_DIVU, UDivInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_DIVS, SDivInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_MODU, URemInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_MODS, SRemInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_AND, AndInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_OR, OrInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_XOR, XorInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_LSL, LogicShiftLeftInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_LSR, LogicShiftRightInstruction);
            DEFINE_BINARY_INSTRUCTION(MLIL_ASR, ArithmeticShiftRightInstruction);
            #undef DEFINE_BINARY_INSTRUCTION

            #define DEFINE_UNARY_INSTRUCTION(bn_opcode, InstructionType) \
            case BNMediumLevelILOperation::bn_opcode: { \
                    newInst = InstructionType::create( \
                        InstructionType::AfterInstruction(placeholderInst),\
                        InstructionType::BitWidth(expr.size * 8),\
                        getTranslatedReadOperand(expr.GetOperands()[0].GetExpr(), placeholderInst)\
                        );\
                    break;\
                }
            DEFINE_UNARY_INSTRUCTION(MLIL_ZX, UnsignedExtendInstruction);
            DEFINE_UNARY_INSTRUCTION(MLIL_SX, SignedExtendInstruction);
            DEFINE_UNARY_INSTRUCTION(MLIL_NOT, NotInstruction);
            DEFINE_UNARY_INSTRUCTION(MLIL_NEG, NegInstruction);
            #undef DEFINE_UNARY_INSTRUCTION

            case BNMediumLevelILOperation::MLIL_SET_VAR_SSA:
            case BNMediumLevelILOperation::MLIL_SET_VAR_ALIASED:  // FIXME: MLIL_SET_VAR_ALIASED should not be translated like this
            {
                newInst = NopInstruction::create(
                        NopInstruction::AfterInstruction(placeholderInst)
                        );
                auto movInst  = MovInstruction::create(
                    MovInstruction::AfterInstruction(placeholderInst),
                    MovInstruction::BitWidth(expr.GetSourceExpr().size * 8),
                    getTranslatedReadOperand(expr.GetSourceExpr(), placeholderInst)
                );
                writeSSAvariable(expr.GetDestSSAVariable(), movInst);
                break;
            };
            case BNMediumLevelILOperation::MLIL_VAR_SSA:
            //case BNMediumLevelILOperation::MLIL_VAR_ALIASED:  // FIXME: MLIL_VAR_ALIASED should not be translated like this
            {
                newInst  = MovInstruction::create(
                    MovInstruction::AfterInstruction(placeholderInst),
                    MovInstruction::BitWidth(getBitWidthOfSSAVariable(ctx, expr.GetSourceSSAVariable())),
                    getTranslatedReadOperand(expr.GetSourceSSAVariable(), placeholderInst)
                );
                break;
            };

            //FIXME
            DEFINE_OPCODE_INSTRUCTION(MLIL_VAR_SSA_FIELD, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_VAR_ALIASED_FIELD, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_SET_VAR_SSA_FIELD, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_SET_VAR_ALIASED_FIELD, UndefiendInstruction);



            case BNMediumLevelILOperation::MLIL_LOAD_SSA:
            {
                newInst = LoadInstruction::create(
                        LoadInstruction::AfterInstruction(placeholderInst),
                        LoadInstruction::BitWidth(expr.size * 8),
                        getTranslatedReadOperand(expr.GetSourceExpr(), placeholderInst)
                        );
                break;
            };
            case BNMediumLevelILOperation::MLIL_LOAD_STRUCT_SSA:
            {
                auto imm = ctx.module.lock()->getConstantInt(expr.GetDestExpr().size * 8, expr.GetOffset());
                auto ptr = AddInstruction::create(
                        LoadInstruction::BeforeInstruction(placeholderInst),
                        LoadInstruction::BitWidth(expr.size * 8),
                        getTranslatedReadOperand(expr.GetSourceExpr(), placeholderInst),
                        imm
                        );
                newInst = LoadInstruction::create(
                        LoadInstruction::AfterInstruction(placeholderInst),
                        LoadInstruction::BitWidth(expr.size * 8),
                        ptr
                        );
                break;
            };
            case BNMediumLevelILOperation::MLIL_STORE_SSA:
            {
                newInst = StoreInstruction::create(
                        StoreInstruction::AfterInstruction(placeholderInst),
                        StoreInstruction::BitWidth(expr.size * 8),
                        getTranslatedReadOperand(expr.GetDestExpr(), placeholderInst),
                        getTranslatedReadOperand(expr.GetSourceExpr(), placeholderInst)
                        );
                break;
            };
            case BNMediumLevelILOperation::MLIL_STORE_STRUCT_SSA:
            {
                auto imm = ctx.module.lock()->getConstantInt(expr.GetDestExpr().size * 8, expr.GetOffset());
                auto ptr = AddInstruction::create(
                        LoadInstruction::BeforeInstruction(placeholderInst),
                        LoadInstruction::BitWidth(expr.size * 8),
                        getTranslatedReadOperand(expr.GetSourceExpr(), placeholderInst),
                        imm
                        );
                newInst = StoreInstruction::create(
                        StoreInstruction::AfterInstruction(placeholderInst),
                        StoreInstruction::BitWidth(expr.size * 8),
                        ptr,
                        getTranslatedReadOperand(expr.GetSourceExpr(), placeholderInst)
                        );
                break;
            };
            DEFINE_OPCODE_INSTRUCTION(MLIL_CALL_OUTPUT_SSA, NopInstruction);

            case BNMediumLevelILOperation::MLIL_CALL_UNTYPED_SSA:
            case BNMediumLevelILOperation::MLIL_CALL_SSA:
            case BNMediumLevelILOperation::MLIL_TAILCALL_SSA:
            case BNMediumLevelILOperation::MLIL_TAILCALL_UNTYPED_SSA:
                {
                    bool returnTheFunction = (expr.operation == MLIL_TAILCALL_UNTYPED_SSA || expr.operation == MLIL_TAILCALL_SSA);
                    auto target = getTranslatedReadOperand(expr.GetDestExpr(), placeholderInst);
                    auto outputSSAVars = expr.GetOutputSSAVariables();
                    auto bitSize = 0;
                    switch (outputSSAVars.size()) {
                        case 0:
                            bitSize = 0;
                            break;
                        case 1:
                            bitSize = getBitWidthOfSSAVariable(ctx, outputSSAVars[0]);
                            break;
                        default:
                            bitSize = getBitWidthOfSSAVariable(ctx, outputSSAVars[0]);
                            WARN("call inst have multiple output");
                            break;
                    }
                    auto callInst = CallInstruction::create(
                            CallInstruction::AfterInstruction(placeholderInst),
                            CallInstruction::BitWidth(bitSize)
                            );

                    //Target
                    callInst->appendOperands(target);
                    for (auto operand: expr.GetParameterExprs()) {
                        callInst->appendOperands(getTranslatedReadOperand(operand, placeholderInst));
                    }

                    // we should create the return instruction (if exist) first..
                    if (returnTheFunction) {
                        switch (outputSSAVars.size()) {
                            case 0:
                                newInst = ReturnInstruction::create(
                                    ReturnInstruction::AfterInstruction(callInst)
                                        );
                            case 1:
                            default:
                                newInst = ReturnInstruction::create(
                                    ReturnInstruction::AfterInstruction(callInst),
                                    callInst
                                        );
                                break;
                        }
                    } else {
                        newInst = NopInstruction::create(
                            NopInstruction::AfterInstruction(callInst)
                            );
                    }

                    // set return value of callinst ssa variable after callinst, before return inst
                    switch (outputSSAVars.size()) {
                        case 0:
                            break;
                        case 1:
                        default:
                            writeSSAvariable(outputSSAVars[0], callInst);
                            break;
                    }
                    break;
                }

            DEFINE_OPCODE_INSTRUCTION(MLIL_NORET, UnreachableInstruction);
            case BNMediumLevelILOperation::MLIL_RET:
                {
                    auto list = expr.GetSourceExprs();
                    newInst = ReturnInstruction::create(
                            ReturnInstruction::AfterInstruction(placeholderInst)
                            );
                    if (list.size()) {
                        newInst->appendOperands(
                                getTranslatedReadOperand(list[0], placeholderInst)
                                );
                    }
                    break;
                }
            case BNMediumLevelILOperation::MLIL_CALL_PARAM:
            case BNMediumLevelILOperation::MLIL_CALL_OUTPUT:
                FATAL("non-ssa binary ninja opcode?");
                break;
            case BNMediumLevelILOperation::MLIL_ADC:
                {
                    auto addInst = AddInstruction::create(
                        AddInstruction::AfterInstruction(placeholderInst),
                        AddInstruction::BitWidth(expr.size * 8),
                        getTranslatedReadOperand(expr.GetLeftExpr(), placeholderInst),
                        getTranslatedReadOperand(expr.GetRightExpr(), placeholderInst)
                        );
                    newInst = AddInstruction::create(
                        AddInstruction::AfterInstruction(placeholderInst),
                        AddInstruction::BitWidth(expr.size * 8),
                        addInst,
                        getTranslatedReadOperand(expr.GetCarryExpr(), placeholderInst)
                        );
                    break;
                }
            case BNMediumLevelILOperation::MLIL_SBB:
                {
                    auto subInst = SubInstruction::create(
                        SubInstruction::AfterInstruction(placeholderInst),
                        SubInstruction::BitWidth(expr.size * 8),
                        getTranslatedReadOperand(expr.GetLeftExpr(), placeholderInst),
                        getTranslatedReadOperand(expr.GetRightExpr(), placeholderInst)
                        );
                    newInst = SubInstruction::create(
                        SubInstruction::AfterInstruction(placeholderInst),
                        SubInstruction::BitWidth(expr.size * 8),
                        subInst,
                        getTranslatedReadOperand(expr.GetCarryExpr(), placeholderInst)
                        );
                    break;
                }
            case BNMediumLevelILOperation::MLIL_ADDRESS_OF:
                {
                    newInst = MovInstruction::create(
                        MovInstruction::AfterInstruction(placeholderInst),
                        MovInstruction::BitWidth(expr.size * 8),
                        getMemoryVariableAllocInst(ctx, expr.GetSourceVariable())
                    );
                    break;
                }
            case BNMediumLevelILOperation::MLIL_ADDRESS_OF_FIELD:
                {
                    auto imm = ctx.module.lock()->getConstantInt(expr.size * 8, expr.GetOffset());
                    newInst = AddInstruction::create(
                        AddInstruction::AfterInstruction(placeholderInst),
                        AddInstruction::BitWidth(expr.size * 8),
                        getMemoryVariableAllocInst(ctx, expr.GetSourceVariable()),
                        imm
                    );
                    break;
                }
            case BNMediumLevelILOperation::MLIL_VAR_PHI:
                {
                    auto phiInst = PhiInstruction::create(
                            PhiInstruction::AfterInstruction(placeholderInst),
                            PhiInstruction::BitWidth(getBitWidthOfSSAVariable(ctx, expr.GetDestSSAVariable()))
                            );
                    for (auto operand: expr.GetSourceSSAVariables()) {
                        FATAL_UNLESS(operand != expr.GetDestSSAVariable());
                        phiInst->appendOperands(getTranslatedReadOperand(operand, placeholderInst));
                    }
                    writeSSAvariable(expr.GetDestSSAVariable(), phiInst);

                    newInst = NopInstruction::create(
                            NopInstruction::AfterInstruction(placeholderInst)
                            );
                    break;
                }

            case BNMediumLevelILOperation::MLIL_IMPORT:
            case BNMediumLevelILOperation::MLIL_CONST:
            case BNMediumLevelILOperation::MLIL_CONST_PTR:
                {
                    //FIXME: cannot use expr.size * 8
                    auto imm = ctx.module.lock()->getConstantInt(64, expr.GetConstant());
                    FATAL_UNLESS(imm);
                    newInst = MovInstruction::create(
                        MovInstruction::AfterInstruction(placeholderInst),
                        MovInstruction::BitWidth(imm->getBitWidth()),
                        imm
                        );
                    break;
                }
            DEFINE_OPCODE_INSTRUCTION(MLIL_MEM_PHI, NopInstruction);



            DEFINE_OPCODE_INSTRUCTION(MLIL_ROL, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_ROR, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_RLC, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_RRC, UndefiendInstruction);
            //floating instruction are translated to undefined instruction
            DEFINE_OPCODE_INSTRUCTION(MLIL_CEIL, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FLOOR, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FCMP_E, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FCMP_GE, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FCMP_GT, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FCMP_LE, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FCMP_LT, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FCMP_NE, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FCMP_O, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FCMP_UO, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FABS, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FADD, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FDIV, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FLOAT_CONST, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FLOAT_CONV, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FMUL, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FNEG, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FSQRT, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FSUB, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_FTRUNC, UndefiendInstruction);


            #define DEFINE_COMPARE_INSTRUCTION(bn_opcode, InstructionType, changeLeftRight, addNot) \
            case BNMediumLevelILOperation::bn_opcode: { \
                    auto left = getTranslatedReadOperand(expr.GetLeftExpr(), placeholderInst); \
                    auto right = getTranslatedReadOperand(expr.GetRightExpr(), placeholderInst); \
                    if (changeLeftRight) { \
                        std::swap(left, right); \
                    }\
                    Instruction * cmpInst = InstructionType::create( \
                        InstructionType::AfterInstruction(placeholderInst),\
                        InstructionType::BitWidth(1),\
                        left, \
                        right\
                        );\
                    if (addNot) { \
                        cmpInst = NotInstruction::create( \
                            NotInstruction::AfterInstruction(cmpInst), \
                            NotInstruction::BitWidth(1), \
                            cmpInst \
                        ); \
                    } \
                    newInst = UnsignedExtendInstruction::create( \
                        InstructionType::AfterInstruction(cmpInst), \
                        InstructionType::BitWidth(expr.size * 8), \
                        cmpInst \
                    ); \
                    break;\
                }

            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_E, EqualWithInstruction, false, false);
            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_NE, EqualWithInstruction, false, true);
            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_SLT, SignedLessThanInstruction, false, false);
            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_SGT, SignedLessThanInstruction, true, false);
            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_SLE, SignedLessThanInstruction, true, true);
            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_SGE, SignedLessThanInstruction, false, true);

            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_ULT, UnsignedLessThanInstruction, false, false);
            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_UGT, UnsignedLessThanInstruction, true, false);
            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_ULE, UnsignedLessThanInstruction, true, true);
            DEFINE_COMPARE_INSTRUCTION(MLIL_CMP_UGE, UnsignedLessThanInstruction, false, true);

            #undef DEFINE_COMPARE_INSTRUCTION

            DEFINE_OPCODE_INSTRUCTION(MLIL_BP, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_TRAP, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_UNDEF, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_UNIMPL, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_UNIMPL_MEM, UndefiendInstruction);

            //FIXME: the follow basic block terminating instruction is not defined yet..
            //DEFINE_OPCODE_INSTRUCTION(MLIL_GOTO, UnreachableInstruction);
            case BNMediumLevelILOperation::MLIL_GOTO:
                {
                    FATAL_UNLESS(ctx.instId2Instruction.contains(expr.GetTarget()));
                    auto target = ctx.instId2Instruction[expr.GetTarget()]->getParent();
                    newInst = UnconditionalBranchInstruction::create(
                            UnconditionalBranchInstruction::AfterInstruction(placeholderInst),
                            target
                            );
                    break;
                }

            case BNMediumLevelILOperation::MLIL_IF:
                {
                    FATAL_UNLESS(ctx.instId2Instruction.contains(expr.GetTrueTarget()));
                    auto trueTarget = ctx.instId2Instruction[expr.GetTrueTarget()]->getParent();
                    FATAL_UNLESS(ctx.instId2Instruction.contains(expr.GetFalseTarget()));
                    auto falseTarget = ctx.instId2Instruction[expr.GetFalseTarget()]->getParent();
                    newInst = ConditionalBranchInstruction::create(
                            ConditionalBranchInstruction::AfterInstruction(placeholderInst),
                            getTranslatedReadOperand(expr.GetConditionExpr(), placeholderInst),
                            trueTarget,
                            falseTarget
                            );
                    break;
                }

            //TODO: JUMP_TO -> SwitchInstruction
            DEFINE_OPCODE_INSTRUCTION(MLIL_JUMP, UndefiendInstruction);
            DEFINE_OPCODE_INSTRUCTION(MLIL_JUMP_TO, UndefiendInstruction);


            default:
                FATAL("unhandled binary ninja opcode? %d", expr.operation);
                break;
            #undef DEFINE_OPCODE_INSTRUCTION
        }

        if (newInst) {
            replaceWith(expr_id, newInst);
            /*
            placeholderInst->replaceUsesWith(newInst);
            placeholderInst->removeFromParent();
            ctx.exprId2Instruction[expr_id] = newInst;
            */
        } else {
            WARN("Inst cannot be translated? PlaceHolder kept as Undefined");
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
        //auto mlil = function->GetMediumLevelILIfAvailable();
        auto mlil = function->GetMediumLevelIL();
        if (!mlil) {
            LOG_INFO << "lifting function " << std::hex << function->GetStart() << "failed, skip";
            continue;
        }
        LOG_INFO << "lifting function " << std::hex << function->GetStart() << " " << mlil.GetPtr() << "start";
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
