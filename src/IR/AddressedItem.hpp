#pragma once
#include <map>
#include <memory>
#include <optional>

#include "IR/Value.hpp"
#include "common.hpp"

template <typename T>
class AddressedConatinerMixin {
	using Container = std::map<uaddr_t, std::shared_ptr<T>>;

public:
	T * getItemByAddress(uaddr_t addr) {
		auto it = _container.find(addr);
		assert(it != _container.end());
		return it->second.get();
	}
	std::shared_ptr<T> popItemByAddress(uaddr_t addr) {
		auto node_handle = _container.extract(addr);
		assert(not node_handle.empty());
		auto retv = node_handle.mapped();
		retv->setParent(nullptr);
		return retv;
	}

	void addAddressedItem(uaddr_t address, std::shared_ptr<T> value);
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
template <typename T>
class AddressedMixin {
public:
	static inline std::shared_ptr<T> create() {
		std::shared_ptr<T> retv = std::shared_ptr<T>(new T());
		return retv;
	}
	static inline std::shared_ptr<T> create(uaddr_t address) {
		auto retv = create();
		retv->setAddress(address);
		return retv;
	}
	bool hasSetAddress() const {
		return this->address_.has_value();
	}
	virtual void setAddress(uaddr_t addr) {
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
class AddressedWithParentMixin: virtual public AddressedMixin<T>, virtual public Value {
public:
	static inline std::shared_ptr<T> create() {
		std::shared_ptr<T> retv = std::shared_ptr<T>(new T());
		return retv;
	}
	static inline std::shared_ptr<T> create(uaddr_t address) {
		auto retv = create();
		retv->setAddress(address);
		return retv;
	}
	virtual void setAddress(uaddr_t addr) override;
	static inline std::shared_ptr<T> create(Parent *parent, uaddr_t address) {
		auto retv = create(address);
		parent->addAddressedItem(retv);
		return retv;
	}
	static inline std::shared_ptr<T> create(std::weak_ptr<Parent> parent, uaddr_t address) {
		Parent * p = parent.lock().get();
		return create(p, address);
	}

	AddressedWithParentMixin() {};
	AddressedWithParentMixin(const AddressedWithParentMixin&) = delete;
	AddressedWithParentMixin(AddressedWithParentMixin&& other) {
		this->parent_ = other.parent_;
		this->address_ = other.address_;
		other.parent_ = nullptr;
		other.address_ = 0;
	}

	void setParent(Parent * parent) {
		std::shared_ptr<T> self = std::dynamic_pointer_cast<T>(shared_from_this());
		assert(self);
		if (this->parent_) {
			auto self = this->parent_->popItemByAddress(this->getAddress());
			assert(self.get() == this);
		}

		this->parent_ = parent;
		if (parent)
			parent->addAddressedItem(self);
	}

	Parent * getParent() {
		assert(not this->parent_);
		return this->parent_;
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

extern template class AddressedWithParentMixin<class Function, class BasicBlock>;
extern template class AddressedWithParentMixin<class Module, class Function>;
extern template class AddressedConatinerMixin<class BasicBlock>;
