#pragma once
#include <cstdint>
class Module;
class Function;

template <typename Self> class AnalysisInfoMixin {
public:
    // used as &Pass::ID, to represet a class
    static uintptr_t getClassId() {
        return static_cast<uintptr_t>(&ID);
    }
private:
    static struct {} ID;
};

class PassManager {
public:
    template<AnalysisT> std::shared_ptr<AnalysisT> getAnalysis(Function *) {
    }

};
