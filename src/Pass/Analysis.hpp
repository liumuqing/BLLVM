#pragma once
#include <cstdint>
class Module;
class Function;


class PassManager {
public:
    template<AnalysisT> std::shared_ptr<AnalysisT> getAnalysis(Function *) {
    }

};
