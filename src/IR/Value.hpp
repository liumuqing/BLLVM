#pragma once
#include <memory>
#include <list>

#include "IR/Use.hpp"
class Value : virtual public std::enable_shared_from_this<Value>{
public:
	virtual std::shared_ptr<const Value> shared_from_this() const{
		return std::enable_shared_from_this<Value>::shared_from_this();
	}
	virtual std::shared_ptr<Value> shared_from_this() {
		return std::enable_shared_from_this<Value>::shared_from_this();
	}
	virtual std::weak_ptr<const Value> weak_from_this() const{
		return std::enable_shared_from_this<Value>::weak_from_this();
	}
	virtual std::weak_ptr<Value> weak_from_this() {
		return std::enable_shared_from_this<Value>::weak_from_this();
	}
	virtual ~Value() {};
private:
	Use *useHead_ = nullptr;

friend class Use;
};
