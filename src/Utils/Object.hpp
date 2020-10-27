#include <memory>
class Object: virtual public std::enable_shared_from_this<Object> {
public:
    template <typename T = Object>
    std::shared_ptr<T> shared_from_this(){
        auto retv = std::dynamic_pointer_cast<T>(std::enable_shared_from_this<Object>::shared_from_this());
        FATAL_UNLESS(retv);
        return retv;
    }
    template <typename T = Object>
    std::shared_ptr<const T> shared_from_this() const {
        auto retv = std::dynamic_pointer_cast<T>(std::enable_shared_from_this<Object>::shared_from_this());
        FATAL_UNLESS(retv);
        return retv;
    }
    template <typename T = Object>
    std::weak_ptr<const T> weak_from_this() const{
        return std::weak_ptr<T>(shared_from_this<T>());
    }
    template <typename T = Object>
    std::weak_ptr<T> weak_from_this() {
        return std::weak_ptr<T>(shared_from_this<T>());
    }
    virtual ~Object() {};
};

