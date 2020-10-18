#include "IR/AddressedItem.hpp"
#include "IR/Module.hpp"
#include "IR/Function.hpp"
#include "IR/BasicBlock.hpp"
template <typename T>
std::shared_ptr<T> ListConatiner<T>::remove(ListContainerItem<T> *item) {

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
size_t WithWidthMixin::getBitWidth() const {
	return width_;
}
void WithWidthMixin::setBitWidth(size_t width) {
	FATAL_UNLESS(width == 0 || width == 1 || width % 8 == 0);
	width_ = width;
}
size_t WithWidthMixin::getByteWidth() const {
	FATAL_UNLESS(width_ % 8 != 0);
	return width_ / 8;
}
void WithWidthMixin::setByteWidth(size_t width) {
	width_ = width * 8;
}

template class ListConatiner<BasicBlock>;
template class ListConatiner<Function>;
template class ListConatiner<Instruction>;
//template class ListConatiner<Function, BasicBlock>;
