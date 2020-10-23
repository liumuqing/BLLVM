#include "IR/Use.hpp"

#include "IR/Value.hpp"

void Use::setValue(Value * newValue) {
    if (this->value.has_value()) {
        this->removeFromList();
        this->value = std::nullopt;
    }

    if (not newValue) {
        return;
    }
    this->value = newValue->weak_from_this();
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
    this->value = std::nullopt;
}
