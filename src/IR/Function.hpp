#pragma once
#include "IR/Module.hpp"
#include "IR/Value.hpp"
#include "IR/AddressedItem.hpp"
#include "common.hpp"
class Module;
class BasicBlock;
class Parameter;
class Function:
    virtual public Value,
    virtual public AddressableListContainerItem<Function>,
    virtual public AddressableListConatiner<BasicBlock>
{
public:
    virtual ~Function() {};

    uaddr_t getStart() const {
        return getAddress();
    }
    void setStart(uaddr_t addr) {
        setAddress(addr);
    }

    Parameter * createAndAppendParameter(size_t bitSize);

    Parameter * getParameter(size_t index) const {
        FATAL_UNLESS(index >= 0 && index < params_.size());
        return params_[index].get();
    }
    size_t countOfParamter() const {
        return this->params_.size();
    }
protected:
    Function() {}
    Function(const Function&) = delete;
private:
    std::vector<std::shared_ptr<Parameter>> params_;

friend class ListContainerItem<Function>;
};

class Parameter:
    virtual public Value,
    virtual public WithWidthMixin,
    virtual public WithParentMixin
{
public:
    virtual ~Parameter(){};
    virtual void setBitWidth(size_t bitWidth) override;
    virtual size_t getIndex() const;
private:
    virtual void setIndex(size_t bitWidth);
    Parameter() {};
    Parameter(const Parameter&) = delete;
    Parameter(Parameter&&) = delete;
    size_t index_ = -1;
friend class Function;
};
