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

#include "Pass/PassManager.hpp"
#include "Pass/ValidateModulePass.hpp"

#include <unordered_map>


#include "Pass/PrintModulePass.hpp"
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

	auto pm = std::shared_ptr<PassManager>(new PassManager());
	//pm->runPassOn<PrintModulePass>(module.get());
	printf("validated :%d", pm->getAnalysisOf<ValidateModulePass>(module.get())->isValidated());
}
