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

template <typename T> class ListConatiner;
template <typename T> class AddressableListConatiner;
template <typename Self, typename Parent> class ListContainerItem;
template <typename Self, typename Parent> class AddressableListContainerItem;

template <typename Self, typename Parent>
class WithParentMixin {
public:
	Parent * getParent() const {
		if (not this->parent_.has_value()) {
			return nullptr;
		}
		FATAL_UNLESS(this->parent_.has_value());
		FATAL_UNLESS(not this->parent_.value().expired());
		return this->parent_.value().lock().get();
	}

protected:
	//these method must be protected...
	//they are used only by ListConatiner
	void setParent(std::nullptr_t parent) {
		this->parent_ = std::nullopt;
	}
	void setParent(Parent * parent) {
		if (parent == nullptr) {
			this->parent_ = std::nullopt;
			return;
		}
		FATAL_UNLESS(not this->parent_.has_value());
		this->parent_ = std::dynamic_pointer_cast<Parent>(parent->shared_from_this());
		FATAL_UNLESS(this->parent_);
	}
	void setParent(Value * parent) {
		FATAL_UNLESS(not(parent and not dynamic_cast<Parent*>(parent)));
		setParent(dynamic_cast<Parent*>(parent));
	}
private:
	std::optional<std::weak_ptr<Parent>> parent_;
};
template <typename Self, typename Parent>
class ListContainerItem:
	virtual public WithParentMixin<Self, Parent>{
public:
	static inline std::shared_ptr<Self> create() {
		std::shared_ptr<Self> retv = std::shared_ptr<Self>(new Self());
		FATAL_UNLESS(retv);
		return retv;
	}
	static inline std::shared_ptr<Self> create(Parent* parent) {
		auto retv = create();
		parent->push_back(retv);
		return retv;
	}
	ListContainerItem() {}
	typename std::list<std::shared_ptr<Self>>::iterator getIterInParent() const {
		FATAL_UNLESS(iter_->get() == this);
		return iter_;
	}
	std::shared_ptr<Self> removeFromParent() {
		FATAL_UNLESS(this->getParent());
		auto retv = this->getParent()->remove(dynamic_cast<Self*>(this));
		return retv;
	}


private:
	ListContainerItem(const ListContainerItem&) = delete;
	ListContainerItem(ListContainerItem&&) = delete;
	typename std::list<std::shared_ptr<Self>>::iterator iter_;

	void setIter(typename std::list<std::shared_ptr<Self>>::iterator iter) {
		iter_ = iter;
	}
template <typename S>
friend class ListConatiner;
};

class AddressableMixin {
public:
	uaddr_t getAddress() const {
		FATAL_UNLESS(hasSetAddress());
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

template <typename T>
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
		return Base::erase(iter);
	}

	auto insert(iterator insertPoint, std::shared_ptr<T> item) {
		auto iter = Base::insert(insertPoint, item);

		FATAL_UNLESS(dynamic_cast<Value*>(this));
		item->setParent(dynamic_cast<Value*>(this));
		item->setIter(iter);
		return iter;
	}


	virtual std::shared_ptr<T> remove(T *item);

	void push_back(std::shared_ptr<T> item) {
		if (item->getParent()) {
			FATAL("this item already has a parent, cannot be pushed....");
		}
		Base::push_back(item);
		auto iter = Base::end();
		iter--;
		FATAL_UNLESS(item->getParent() == nullptr);
		FATAL_UNLESS(dynamic_cast<Value*>(this));
		item->setParent(dynamic_cast<Value*>(this));
		item->setIter(iter);
	}

	virtual ~ListConatiner() {}
};


template <typename T>
class AddressableListConatiner: public ListConatiner<T> {
	using Base = ListConatiner<T>;
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
};

template <typename Self, typename Parent>
class AddressableListContainerItem: virtual public AddressableMixin, virtual public ListContainerItem<Self, Parent>{
public:
	static inline std::shared_ptr<Self> create() {
		return ListContainerItem<Self, Parent>::create();
	}
	static inline std::shared_ptr<Self> create(Parent* parent) {
		return ListContainerItem<Self, Parent>::create(parent);
	}
	static inline std::shared_ptr<Self> create(Parent *parent, uaddr_t address) {
		auto retv = create(address);
		parent->push_back(retv);
		return retv;
	}
	static inline std::shared_ptr<Self> create(uaddr_t address) {
		auto retv = ListContainerItem<Self, Parent>::create();
		retv->setAddress(address);
		return retv;
	}
	static inline std::shared_ptr<Self> create(std::shared_ptr<Parent> parent, uaddr_t address) {
		return create(parent.get(), address);
	}

	AddressableListContainerItem() {};
	AddressableListContainerItem(const AddressableListContainerItem&) = delete;

	virtual ~AddressableListContainerItem() {
	}

template <typename C>
friend class AddressableListConatiner;
};

/*
extern template class ListConatiner<class BasicBlock>;
extern template class ListConatiner<class Function>;
extern template class ListConatiner<class BasicBlock>;
*/
