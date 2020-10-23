#include "IR/BasicBlock.hpp"
#include "IR/Instruction.hpp"
#include "IR/Function.hpp"
#include "common.hpp"
std::shared_ptr<Instruction> BasicBlock::removeInstruction(Instruction * inst) {
    return remove(inst);
}
void BasicBlock::insertInstructionAt(Conatiner::iterator it, std::shared_ptr<Instruction> inserted) {
    if (inserted->getParent()) {
        FATAL("inst that to be inserted, already blongs to an bbl");
    }
    BasicBlock::Conatiner::insert(it, inserted);
}
void BasicBlock::insertInstructionAfter(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted) {
    FATAL_UNLESS(insertPoint->getParent() == this);

    auto it = insertPoint->getIterInParent();
    it++;
    insertInstructionAt(it, inserted);
}
void BasicBlock::insertInstructionBefore(const Instruction * insertPoint, std::shared_ptr<Instruction> inserted) {
    FATAL_UNLESS(insertPoint->getParent() == this);

    auto it = insertPoint->getIterInParent();
    insertInstructionAt(it, inserted);
}
