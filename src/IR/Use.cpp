#include "IR/Use.hpp"

#include "IR/Value.hpp"

void Use::setValue(Value * newValue) {
	if (this->value) {
		this->removeFromList();
		this->value = nullptr;
	}
	this->value = newValue;
	if (this->value) {
		this->addToList(&newValue->useHead_);
	}
}

void Use::removeFromList() {
	*this->prev = this->next;
	if (this->next) {
		this->next->prev = this->prev;
	}
	this->prev = nullptr;
	this->next = nullptr;
	this->value = nullptr;
}
