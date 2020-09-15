#pragma once
#include <cstddef> //size_t
#include <memory>
#include <vector>

#include "common.hpp"

class Value;
class User;
class Use {
	public:
		Use (User *parent) {
			assert(parent != nullptr);
			this->parent = parent;
		}

		Use(const Use&) = delete;
		Use(Use&& other) {
			this->parent = other.parent;
			this->value = other.value;
			this->next = other.next;
			this->prev = other.prev;
			if (this->next) {
				assert(this->value);
				this->next->prev = &this->next;
			}
			if (this->prev) {
				assert(this->value);
				(*this->prev) = this;
			}
			//We should set parent null here...
			//But ~Use will assert(parent != nullptr)
			//other.parent = nullptr;
			other.value = nullptr;
			other.next = nullptr;
			other.prev = nullptr;
		}

		~Use() {
			// No one can change parent, so this value must not be nullptr;
			assert(this->parent != nullptr);
			if (this->value) {
				removeFromList();
			}
		}

		Value * getValue() {
			return this->value;
		}
		void setValue(Value * newValue);

		User * getUser() {
			return this->parent;
		}

		void setUser(User *user) {
			this->parent = user;
		}

		//Never SetUser....
	protected:
		void removeFromList();

		void addToList(Use **listHead) {
			//make sure a use is never add to a list twice
			//  (you can remove it from List first)
			assert(this->next == nullptr);
			assert(this->prev == nullptr);

			this->next = *listHead;
			if (this->next) {
				this->next->prev = &this->next;
			}
			this->prev = listHead;
			*this->prev = this;
		}
	private:
		User * parent = nullptr;
		Value * value = nullptr;
		Use * next = nullptr;
		Use ** prev = nullptr;
};