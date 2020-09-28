#include "IR/AddressedItem.hpp"
#include "IR/Module.hpp"
#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"

template <typename Self, typename T>
void AddressedConatinerMixin<Self, T>::addAddressedItem(uaddr_t address, std::shared_ptr<T> value) {
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

template <typename Self, typename T>
std::shared_ptr<T> ListConatiner<Self, T>::remove(T *item) {
		assert(item);
		auto extractNode = itemIteratorMapping.extract(item);
		assert(not extractNode.empty());
		if (extractNode.empty()) {
			return nullptr;
		}

		auto iter = extractNode.mapped();
		auto retv = *iter;

		Base::erase(iter);
		assert(retv.get() == item);
		retv->setParent(nullptr);
		return retv;
	}

template class AddressedWithParentMixin<Function, BasicBlock>;
template class AddressedWithParentMixin<Module, Function>;
template class AddressedConatinerMixin<Function, BasicBlock>;

template class ListConatiner<Function, BasicBlock>;
template class ListConatiner<Module, Function>;
//template class ListConatiner<Function, BasicBlock>;
