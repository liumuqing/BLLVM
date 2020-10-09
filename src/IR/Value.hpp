#pragma once
#include <memory>
#include <list>

#include "IR/Use.hpp"
#include "common.hpp"
class Value : virtual public std::enable_shared_from_this<Value>{
public:
	template <typename T = Value>
	std::shared_ptr<T> shared_from_this(){
		auto retv = std::dynamic_pointer_cast<T>(std::enable_shared_from_this<Value>::shared_from_this());
		FATAL_UNLESS(retv);
		return retv;
	}
	template <typename T = Value>
	std::shared_ptr<const T> shared_from_this() const {
		auto retv = std::dynamic_pointer_cast<T>(std::enable_shared_from_this<Value>::shared_from_this());
		FATAL_UNLESS(retv);
		return retv;
	}
	template <typename T = Value>
	std::weak_ptr<const T> weak_from_this() const{
		return std::weak_ptr<T>(shared_from_this<T>());
	}
	template <typename T = Value>
	std::weak_ptr<T> weak_from_this() {
		return std::weak_ptr<T>(shared_from_this<T>());
	}
	virtual ~Value() {};

	//replace all uses of self to value.
	void replaceUsesWith(Value * value) {
		while (useHead_) {
			auto p = useHead_;
			p->setValue(value);
		}
	}

	bool isUsed() const {
		return useHead_;
	}
private:
	Use *useHead_ = nullptr;


friend class Use;
};
