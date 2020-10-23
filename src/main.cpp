//binaryninja
#include <plog/Severity.h>
#include <binaryninjaapi.h>
#include <mediumlevelilinstruction.h>

#include <list>
#include <memory>
#include <vector>

#include <argparse/argparse.hpp>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/FuncMessageFormatter.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include <libgen.h>
#include <dlfcn.h>
#include <binaryninjacore.h>
#include <iostream>
#include <filesystem>

#include "ModuleLoader/BinaryNinjaModuleLoader.hpp"
#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Instruction.hpp"
#include "IR/Constant.hpp"

#include <unordered_map>

// define main as weak symbol, so we can rewrite in test
__attribute__((weak))
int main(int argc, const char *argv[]) {
    plog::ColorConsoleAppender<plog::TxtFormatter> colorAppender;
    plog::init(plog::verbose, &colorAppender);
    //
    auto parser = argparse::ArgumentParser("binary-check3");
    parser.add_argument("-i", "--input")
        .required()
        .help("input path, can be bndb file");

    try {
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
        std::cout << parser;
        exit(0);
    }
    std::string input_path = parser.get<std::string>("-i");
    std::string bndb_path = (input_path.ends_with(".bndb") ? input_path : input_path + ".bndb");

    auto module_loader = std::make_unique<BinaryNinjaModuleLoader>();
    module_loader->open(input_path.c_str());
    auto module = module_loader->lift();

    std::unordered_map<Instruction *, size_t> inst2id;

    auto getId = [&inst2id](Instruction * inst) -> size_t {
        if (inst2id.contains(inst)) {
            return inst2id[inst];
        }
        size_t retv = inst2id.size()+1;
        inst2id[inst] = retv;
        return retv;
    };

    for (auto function: *module) {
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
}
