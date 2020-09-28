#include "IR/AddressedItem.hpp"
#include "IR/Module.hpp"
#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"
template <typename T>
std::shared_ptr<T> ListConatiner<T>::remove(T *item) {

	FATAL_UNLESS(item);

	// item's parent must be this, otherwise this item is not belongs to this.
	FATAL_UNLESS(item->getParent() == this);

	auto iter = item->getIterInParent();
	auto retv = *iter;

	erase(iter);
	FATAL_UNLESS(retv.get() == item);
	retv->setParent(nullptr);
	return retv;
}

//template class AddressableListConatiner<BasicBlock>;

template class ListConatiner<BasicBlock>;
template class ListConatiner<Function>;
template class ListConatiner<Instruction>;
//template class ListConatiner<Function, BasicBlock>;
