#pragma once
#include <memory>
#include <map>
#include <type_traits>
#include "Utils/Object.hpp"
#include "Pass/Pass.hpp"

#include "IR/Function.hpp"
#include "IR/Module.hpp"

#include "common.hpp"

class PassManager: virtual public Object {
public:
    virtual ~PassManager() = default;
    void invalidAllCacheOf(Module * module) {
        moduleAnalysisCache_.erase(module->shared_from_this<Module>());
    }
    void invalidAllCacheOf(Function * function) {
        functionAnalysisCache_.erase(function->shared_from_this<Function>());
    }
    /* 无条件 run一次pass，必要的情况下，清空Cache*/
    template <typename PassT> std::shared_ptr<PassT> runPassOn(Function * target) {
        static_assert(std::is_base_of<FunctionPass, PassT>::value);

        //FIXME: when a function is removed from it's parent(i.e a module), who is responsible to invalid `functionAnalysisCache_`?
        std::shared_ptr<PassT> newPass = std::shared_ptr<PassT>(new PassT());
        newPass->initialize(target, this);
        bool changed = newPass->run();

        if (changed) {
            FATAL_UNLESS((std::is_base_of<Transformation, PassT>::value));
            invalidAllCacheOf(target);
        }
        invalidAllCacheOf(target);
        return newPass;
    };

    template <typename PassT> std::shared_ptr<PassT> runPassOn(Module * target) {
        static_assert(std::is_base_of<ModulePass, PassT>::value);

        //FIXME: when a function is removed from it's parent(i.e a module), who is responsible to invalid `functionAnalysisCache_`?
        std::shared_ptr<PassT> newPass = std::shared_ptr<PassT>(new PassT());
        newPass->initialize(target, this);
        bool changed = newPass->run();
        if (changed) {
            FATAL_UNLESS((std::is_base_of<Transformation, PassT>::value));
            invalidAllCacheOf(target);
        }
        invalidAllCacheOf(target);
        return newPass;
    };

    template <typename PassT> std::shared_ptr<PassT> getAnalysisOf(Module * target) {
        static_assert(std::is_base_of<ModulePass, PassT>::value);
        static_assert(std::is_base_of<Analysis, PassT>::value);
        
        if (not moduleAnalysisCache_.contains(target->shared_from_this<Module>())) {
            goto new_pass;
        }

        if (not moduleAnalysisCache_[target->shared_from_this<Module>()].contains(PassT::ID)) {
            goto new_pass;
        }

        goto return_from_cache;

        new_pass:
        {
            auto new_pass = runPassOn<PassT>(target);
            FATAL_UNLESS(not moduleAnalysisCache_[target->shared_from_this<Module>()].contains(PassT::ID));

            moduleAnalysisCache_[target->shared_from_this<Module>()][PassT::ID] = new_pass;
        }

        return_from_cache:
        auto modulePass = moduleAnalysisCache_[target->shared_from_this<Module>()][PassT::ID];
        FATAL_UNLESS(modulePass);
        auto retv = std::dynamic_pointer_cast<PassT>(modulePass);
        FATAL_UNLESS(retv);;
        return retv;
    }
    
    template <typename PassT> std::shared_ptr<PassT> getAnalysisOf(Function * target) {
        static_assert(std::is_base_of<FunctionPass, PassT>::value);
        static_assert(std::is_base_of<Analysis, PassT>::value);
        
        if (not functionAnalysisCache_.contains(target->shared_from_this<Function>())) {
            goto new_pass;
        }

        if (not functionAnalysisCache_[target->shared_from_this<Function>()].contains(PassT::ID)) {
            goto new_pass;
        }

        goto return_from_cache;

        new_pass:
        {
            auto new_pass = runPassOn<PassT>(target);
            FATAL_UNLESS(not functionAnalysisCache_[target->shared_from_this<Function>()].contains(PassT::ID));

            functionAnalysisCache_[target->shared_from_this<Function>()][PassT::ID] = new_pass;
        }

        return_from_cache:
        auto functionPass = functionAnalysisCache_[target->shared_from_this<Function>()][PassT::ID];
        FATAL_UNLESS(functionPass);
        auto retv = std::dynamic_pointer_cast<PassT>(functionPass);
        FATAL_UNLESS(retv);;
        return retv;
        
    }
    
protected:
    template <typename PassT, typename TargetT> std::shared_ptr<PassT> getNewPassOf(Module * target) {
    }
    template <typename T> struct AnalysisCache: public std::map<PassID, std::shared_ptr<T>> {};
private:
    // we must use shared_ptr insead of weak_ptr as key, since weak_ptr does't support operator <=>
    std::map<std::shared_ptr<Function>, AnalysisCache<FunctionPass>> functionAnalysisCache_;
    std::map<std::shared_ptr<Module>, AnalysisCache<ModulePass>> moduleAnalysisCache_;
};
