#pragma once
#include <map>
#include <memory>
#include <optional>

#include "common.hpp"

template <typename T>
class AddressedConatinerMixin {
	using Container = std::map<uaddr_t, std::shared_ptr<T>>;

public:
	std::shared_ptr<T> popItemByAddress(uaddr_t addr) {
		auto node_handle = _container.extract(addr);
		assert(not node_handle.empty());
		return node_handle.value();
	}

	void addAddressedItem(uaddr_t address, std::shared_ptr<T> value) {
		assert(value);
		if (value->hasSetAddress()) {
			assert(value->getAddress() == address);
		}
		else {
			value->setAddress(address);
		}
		addAddressedItem(value);
	}
	void addAddressedItem(std::shared_ptr<T> value) {
		// value is not nullptr and _container don't have same item
		assert(value && _container.find(value->getAddress()) == _container.end());
		_container[value->getAddress()] = value;
	}

	std::vector<T*> children() {
		std::vector<T*> retv;
		for (auto [k, v]: _container) {
			retv.push_back(v.get());
		}
		return retv;
	}

protected:
	Container _container;
};

class AddressedMixin {
public:
	bool hasSetAddress() const {
		return this->address_.has_value();
	}
	void setAddress(uaddr_t addr) {
		*this->address_ = addr;
		this->address_ = std::make_optional(addr);
		assert(this->address_.has_value());
	}

	uaddr_t getAddress() const {
		assert(this->address_.has_value());
		return this->address_.value();
	}

protected:
	std::optional<uaddr_t> address_ = std::nullopt;
};

template <typename Parent, typename T>
class AddressedWithParentMixin : public AddressedMixin {

public:
	AddressedWithParentMixin() {};
	AddressedWithParentMixin(const AddressedWithParentMixin&) = delete;
	AddressedWithParentMixin(AddressedWithParentMixin&& other) {
		this->parent_ = other->parent_;
		this->address_ = other->address_;
		other->parent_ = nullptr;
		other->address_ = 0;
	}

	std::shared_ptr<T> removeFromParent() {
		assert(parent_);
		auto retv = parent_->popItemByAddress(this->getAddress());
		this->parent_ = nullptr;
		return retv;
	}


	virtual ~AddressedWithParentMixin() {
		// you should remove this item from its parent first.
		assert(not this->parent_);
	}

protected:
	Parent * parent_ = nullptr;

};
