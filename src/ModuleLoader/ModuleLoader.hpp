#pragma once
#include <memory>
#include <tuple>
#include <optional>

#include "IR/Module.hpp"
#include "common.hpp"
class ModuleLoader {
    /*
     * Interface
     */
public:
    virtual void open(const char *) = 0;
    virtual std::shared_ptr<Module> lift() = 0;
    virtual uint8_t getByteAt(uaddr_t addr) = 0;
    virtual bool isBigEndian() {return not isLittleEndian();}
    virtual bool isLittleEndian() final {
        // you should override isBigEndian, but not this method
        return not isBigEndian();
    }
    virtual std::optional<std::string> getSymbolNameAt(uaddr_t addr) = 0;
    virtual ~ModuleLoader() {}
};
