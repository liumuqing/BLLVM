#include "IR/AddressedItem.hpp"
#include "IR/Module.hpp"
#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"

template <typename T>
void AddressedConatinerMixin<T>::addAddressedItem(uaddr_t address, std::shared_ptr<T> value) {
    assert(value);
    if (value->hasSetAddress()) {
        assert(value->getAddress() == address);
    }
    else {
        value->setAddress(address);
    }
    assert(value->hasSetAddress());
    addAddressedItem(value);
}
template <typename Parent, typename T>
void AddressedWithParentMixin<Parent, T>::setAddress(uaddr_t addr) {
    auto parent = getParent();

	// we may have a child node, which don't have parent
	if (parent) {
		auto self = removeFromParent();
		AddressedMixin<T>::setAddress(addr);
		parent->addAddressedItem(self);
	}
	else {
		AddressedMixin<T>::setAddress(addr);
	}
}


template class AddressedWithParentMixin<Function, BasicBlock>;
template class AddressedWithParentMixin<Module, Function>;
template class AddressedConatinerMixin<BasicBlock>;
