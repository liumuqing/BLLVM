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
template <typename Self> class ListContainerItem;
template <typename Self> class AddressableListContainerItem;

class WithWidthMixin{
public:
	// return 0 if not set
	size_t getBitWidth() const;
	virtual void setBitWidth(size_t width);
	size_t getByteWidth() const;
	void setByteWidth(size_t width);
private:
	size_t width_ = 0;
};
class WithParentMixin{
public:
	Value * getParent() const {
		if (not this->parent_.has_value()) {
			return nullptr;
		}
		FATAL_UNLESS(this->parent_.has_value());
		FATAL_UNLESS(not this->parent_.value().expired());
		auto retv = this->parent_.value().lock().get();
		return retv;
	}
	void setParent(Value * parent) {
		if (parent == nullptr) {
			this->parent_ = std::nullopt;
		} else {
			this->parent_ = parent->weak_from_this();
		}
	}
private:
	std::optional<std::weak_ptr<Value>> parent_;
};
template <typename Self>
class ListContainerItem:
	virtual public Value,
	virtual public WithParentMixin {
public:
	static inline std::shared_ptr<Self> create() {
		std::shared_ptr<Self> retv = std::shared_ptr<Self>(new Self());
		FATAL_UNLESS(retv);
		return retv;
	}
	static inline Self* create(ListConatiner<Self> * parent) {
		auto retv = create();
		parent->push_back(retv);
		return retv.get();
	}
	ListContainerItem() {}
	typename std::list<std::shared_ptr<Self>>::iterator getIterInParent() const {
		FATAL_UNLESS(iter_->get() == this);
		return iter_;
	}
	std::shared_ptr<Self> removeFromParent() {
		FATAL_UNLESS(this->getParent());

		auto parent = dynamic_cast<ListConatiner<Self> *>(this->getParent());
		FATAL_UNLESS(parent);

		auto retv = parent->remove(this);
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
class ListConatiner:
	virtual public Value,
	private std::list<std::shared_ptr<T>> {
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


	virtual std::shared_ptr<T> remove(ListContainerItem<T> *item);

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

template <typename Self>
class AddressableListContainerItem: virtual public AddressableMixin, virtual public ListContainerItem<Self>{
public:
	static inline std::shared_ptr<Self> create() {
		return ListContainerItem<Self>::create();
	}
	static inline Self* create(ListConatiner<Self>* parent) {
		return ListContainerItem<Self>::create(parent);
	}
	static inline std::shared_ptr<Self> create(ListConatiner<Self> *parent, uaddr_t address) {
		auto retv = create(address);
		parent->push_back(retv);
		return retv;
	}
	static inline std::shared_ptr<Self> create(uaddr_t address) {
		auto retv = ListContainerItem<Self>::create();
		retv->setAddress(address);
		return retv;
	}
	static inline std::shared_ptr<Self> create(std::shared_ptr<ListConatiner<Self>> parent, uaddr_t address) {
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
