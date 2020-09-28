#pragma once
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#include <list>
#include <set>
#include <type_traits>

#include "IR/Value.hpp"
#include "common.hpp"

template <typename Self, typename T> class ListConatiner;
template <typename Self, typename T> class AddressedConatinerMixin;
template <typename Self, typename Parent> class HasParentMixin;
template <typename Self, typename Parent> class AddressedWithParentMixin;


template <typename Self>
class HasParentBaseMixin {
public:
	HasParentBaseMixin(): parent_({}) {}
	Value * getParent() const {
		if (not this->parent_.has_value()) {
			return nullptr;
		}
		assert(this->parent_.has_value());
		assert(not this->parent_.value().expired());
		return this->parent_.value().lock().get();
	}

private:
	HasParentBaseMixin(const HasParentBaseMixin&) = delete;
	HasParentBaseMixin(HasParentBaseMixin&&) = delete;
	std::optional<std::weak_ptr<Value>> parent_;

protected:
	void setParent(Value * parent) {
		assert(not this->parent_.has_value());
		this->parent_ = parent->weak_from_this();
	}
template <typename C, typename S>
friend class ListConatiner;
};

template <typename Self, typename Parent>
class HasParentMixin: protected HasParentBaseMixin<Self> {
	using Base = HasParentBaseMixin<Self>;
public:
	HasParentMixin(): Base() {}
	Parent * getParent() const {
		auto parent = Base::getParent();
		auto retv = dynamic_cast<Parent*>(parent);

		// make sure the cast success
		assert(not((!retv) && (parent)));

		return retv;
	}

private:
	HasParentMixin(const HasParentMixin&) = delete;
	HasParentMixin(HasParentMixin&&) = delete;

protected:
	void setParent(Parent * parent) {
		return Base::setParent(parent);
	}
template <typename S, typename C>
friend class ListConatiner;
};


template <typename Self>
class AddressableMixin {
public:
	uaddr_t getAddress() const {
		assert(hasSetAddress());
		return this->address_.value();
	}
	void setAddress(uaddr_t address) {
		setAddress_(address);
	}
	bool hasSetAddress() const {
		return this->address_.has_value();
	}
	AddressableMixin(): address_(std::nullopt) {}

private:
	void setAddress_(uaddr_t address) {
		this->address_ = address;
	}
	AddressableMixin(const AddressableMixin&) = delete;
	AddressableMixin(AddressableMixin&&) = delete;
	std::optional<uaddr_t> address_ = {};
};

template <typename Self, typename T>
class ListConatiner: private std::list<std::shared_ptr<T>> {
	using Base = std::list<std::shared_ptr<T>>;
public:
	using iterator = typename Base::iterator;

	auto begin() {
		return Base::begin();
	}

	auto end() {
		return Base::end();
	}
	auto erase(iterator iter) {
		T* ptr = iter->get();
		auto del_iter = itemIteratorMapping.find(ptr);
		assert(del_iter != itemIteratorMapping.end());
		itemIteratorMapping.erase(del_iter);
		return Base::erase(iter);
	}

	virtual std::shared_ptr<T> remove(T *item);

	void push_back(std::shared_ptr<T> item) {
		assert(itemIteratorMapping.find(item.get()) == itemIteratorMapping.end());
		if (item->getParent()) {
			FATAL("this item already has a parent, cannot be pushed....");
		}
		Base::push_back(item);
		assert(item->getParent() == nullptr);
		assert(dynamic_cast<Self*>(this));
		item->setParent(dynamic_cast<Self*>(this));
	}

private:
	std::unordered_map<T *, iterator> itemIteratorMapping;
};

extern template class ListConatiner<class Function, class BasicBlock>;
extern template class ListConatiner<class Module, class Function>;
extern template class ListConatiner<class Function, class BasicBlock>;

template <typename Self, typename T>
class AddressedConatinerMixin: public ListConatiner<Self, T> {
	using Base = ListConatiner<Self, T>;
	using Container = std::map<std::pair<uaddr_t, void *>, typename Base::iterator>;

public:
	T * getItemByAddress(uaddr_t addr) {
		for (auto item: *this) {
			if (item->hasSetAddress() and item->getAddress() == addr) {
				return item.get();
			}
		}
		return nullptr;
	}
	std::shared_ptr<T> popItemByAddress(uaddr_t addr) {
		for (auto iter = this->begin(), E = this->end(); iter != E; ++iter) {
			if ((*iter)->hasSetAddress() and (*iter)->getAddress() == addr) {
				auto retv = *iter;
				this->erase(iter);
				return retv;
			}
		}
		return nullptr;
	}

	void addAddressedItem(uaddr_t address, std::shared_ptr<T> value);
	void addAddressedItem(std::shared_ptr<T> value) {
		// value is not nullptr and _container don't have same item
		//assert(value && _container.find(value->getAddress()) == _container.end());
		Base::push_back(value);
	}
};

template <typename Parent, typename Self>
class AddressedWithParentMixin: public AddressableMixin<Self>, public HasParentMixin<Self, Parent>{
public:
	static inline std::shared_ptr<Self> create() {
		std::shared_ptr<Self> retv = std::shared_ptr<Self>(new Self());
		assert(retv);
		return retv;
	}
	static inline std::shared_ptr<Self> create(uaddr_t address) {
		auto retv = create();
		retv->setAddress(address);
		return retv;
	}
	static inline std::shared_ptr<Self> create(Parent *parent, uaddr_t address) {
		auto retv = create(address);
		parent->addAddressedItem(retv);
		return retv;
	}
	static inline std::shared_ptr<Self> create(std::shared_ptr<Parent> parent, uaddr_t address) {
		return create(parent.get(), address);
	}

	AddressedWithParentMixin() {};
	AddressedWithParentMixin(const AddressedWithParentMixin&) = delete;
public:
	std::shared_ptr<Self> removeFromParent() {
		assert(this->getParent());
		auto retv = this->getParent()->remove(dynamic_cast<Self*>(this));
		return retv;
	}


	virtual ~AddressedWithParentMixin() {
	}

template <typename S, typename C>
friend class AddressedConatinerMixin;
};

extern template class AddressedWithParentMixin<class Function, class BasicBlock>;
extern template class AddressedWithParentMixin<class Module, class Function>;
extern template class AddressedConatinerMixin<class Function, class BasicBlock>;
