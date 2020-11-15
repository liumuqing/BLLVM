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
	void invalid(Pass * pass);

	void invalidAllCacheOf(Module *);
	void invalidAllCacheOf(Function *);
	/* 无条件 run一次pass，必要的情况下，清空Cache*/
	template <typename PassT> std::shared_ptr<PassT> runPassOn(Function * target) {
		static_assert(std::is_base_of<FunctionPass, PassT>::value);

		//FIXME: when a function is removed from it's parent(i.e a module), who is responsible to invalid `functionPassCache_`?
		std::shared_ptr<PassT> newPass = std::shared_ptr(new PassT());
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

		//FIXME: when a function is removed from it's parent(i.e a module), who is responsible to invalid `functionPassCache_`?
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
		
		if (not modulePassCache_.contains(target->weak_from_this<Module>())) {
			goto new_pass;
		}

		if (not modulePassCache_[target->weak_from_this<Module>()].contains(PassT::ID)) {
			goto new_pass;
		}

		goto return_from_cache;

		new_pass:
		{
			auto new_pass = runPassOn<PassT>(target);
			FATAL_UNLESS(not modulePassCache_[target->weak_from_this<Module>()].contains(PassT::ID));

			modulePassCache_[target->weak_from_this<Module>()][PassT::ID] = new_pass;
		}

		return_from_cache:
		return modulePassCache_[target->weak_from_this<Module>()][PassT::ID];
	}
	
	template <typename PassT> std::shared_ptr<PassT> getAnalysisOf(Function * target) {
		static_assert(std::is_base_of<FunctionPass, PassT>::value);
		static_assert(std::is_base_of<Analysis, PassT>::value);
		
		if (not functionPassCache_.contains(target->weak_from_this<Function>())) {
			goto new_pass;
		}

		if (not functionPassCache_[target->weak_from_this<Function>()].contains(PassT::ID)) {
			goto new_pass;
		}

		goto return_from_cache;

		new_pass:
		{
			auto new_pass = runPassOn<PassT>(target);
			FATAL_UNLESS(not functionPassCache_[target->weak_from_this<Function>()].contains(PassT::ID));

			functionPassCache_[target->weak_from_this<Function>()][PassT::ID] = new_pass;
		}

		return_from_cache:
		return functionPassCache_[target->weak_from_this<Function>()][PassT::ID];
	}
	
protected:
	template <typename PassT, typename TargetT> std::shared_ptr<PassT> getNewPassOf(Module * target) {
	}
	template <typename T> struct PassCache: public std::map<PassID, std::shared_ptr<T>> {};
private:
	std::map<std::weak_ptr<Function>, PassCache<FunctionPass>> functionPassCache_;
	std::map<std::weak_ptr<Module>, PassCache<ModulePass>> modulePassCache_;
};
