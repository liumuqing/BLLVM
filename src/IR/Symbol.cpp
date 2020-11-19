#include <memory>
#include <string>
#include "Symbol.hpp"

Symbol * Symbol::create(Module * parent, const char * name) {
    auto r = parent->getSymbol(name, false);
    if (r) {
        return r;
    }
    auto retv = std::shared_ptr<Symbol>(new Symbol());
    retv->symbolName_ = name;
    FATAL_UNLESS(retv->getSymbolName() == retv->symbolName_);
    parent->addSymbol(retv);
    return retv.get();
}

std::string Symbol::getSymbolName() const {
    return this->symbolName_;
}

