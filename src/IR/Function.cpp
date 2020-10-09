#include "IR/Function.hpp"

Parameter * Function::createAndAppendParameter(size_t bitWidth) {
	auto p = std::shared_ptr<Parameter>(new Parameter());
	p->setBitWidth(bitWidth);

	this->params_.push_back(p);
	auto retv = p.get();
	FATAL_UNLESS(retv);

	auto index = (this->params_.size()-1);
	retv->setIndex(index);
	return retv;
}

void Parameter::setBitWidth(size_t bitWidth) {
	FATAL_UNLESS(not this->getParent());

	return WithWidthMixin::setBitWidth(bitWidth);
}
void Parameter::setIndex(size_t index) {
	this->index_ = index;
}

size_t Parameter::getIndex() const {
	return this->index_;
}
