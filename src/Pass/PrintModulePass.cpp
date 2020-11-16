#include "Pass/PrintModulePass.hpp"

#include <unordered_map>
#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Instruction.hpp"
#include "IR/Constant.hpp"

bool PrintModulePass::run() {
    std::unordered_map<Instruction *, size_t> inst2id;
    auto getId = [&inst2id](Instruction * inst) -> size_t {
        if (inst2id.contains(inst)) {
            return inst2id[inst];
        }
        size_t retv = inst2id.size()+1;
        inst2id[inst] = retv;
        return retv;
    };

    for (auto function: *this->getTarget()) {
        inst2id.clear();
        printf("function_%lx\n", function->getAddress());
        for (auto bbl: *function) {
            printf("bbl_%p\n", bbl.get());
            for (auto inst: *bbl) {
                printf("$%zu = %s ", getId(inst.get()), inst->getOpstr());
                for (auto index = 0; index < inst->getNumOperands(); index ++) {
                    auto operand = inst->getOperand(index);
                    if (auto opInst = dynamic_cast<Instruction *>(operand)) {
                        printf("$%zu, ", getId(opInst));
                    }
                    else if (auto opInst = dynamic_cast<ConstantInt *>(operand)) {
                        printf("#%zu, ", opInst->getUnsignedValue());
                    }
                    else if (auto opInst = dynamic_cast<Parameter *>(operand)) {
                        printf("$arg%zu, ", opInst->getIndex());
                    }
                    else if (auto opInst = dynamic_cast<BasicBlock *>(operand)) {
                        printf("bbl_%p, ", opInst);
                    }
                    else {
                        printf("??, ");
                    }
                }
                printf("\n");
            }
        }
    }

	return false;
}
