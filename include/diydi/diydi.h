#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <type_traits>
#include <typeinfo>

namespace diydi {
enum Scope { DEFAULT, SINGLETON };

class already_bound_error : public std::exception {
public:
  already_bound_error(std::string msg) : msg(msg) {}
  const char* what() const throw() { return msg.c_str(); }

private:
  std::string msg;
};

class dependency_resolution_error : public std::exception {
public:
  dependency_resolution_error(std::string msg) : msg(msg) {}
  const char* what() const throw() { return msg.c_str(); }

private:
  std::string msg;
};

class Container {
public:
  Container() {}
  Container(const Container&) = delete;
  Container& operator=(const Container&) = delete;

  template <typename Interface, typename Implementation,
            typename... Dependencies>
  void bind(Scope scope = Scope::DEFAULT) {
    static_assert(std::is_base_of<Interface, Implementation>::value,
                  "Implementation must inherit from Interface");

    int typeID = getTypeID<Interface>();

    if (bindings.count(typeID)) {
      throw already_bound_error(std::string(typeid(Interface).name()) +
                                std::string(" already bound"));
    }

    if (scope == Scope::DEFAULT) {
      bindings[typeID] = [this]() {
        return std::make_shared<Implementation>(getInstance<Dependencies>()...);
      };
    } else if (scope == Scope::SINGLETON) {
      bindings[typeID] = [this]() {
        static std::shared_ptr<Implementation> implementation =
            std::make_shared<Implementation>(getInstance<Dependencies>()...);
        return implementation;
      };
    }
  }

  template <typename Interface> std::shared_ptr<Interface> getInstance() {
    int typeID = getTypeID<Interface>();

    if (!bindings.count(typeID)) {
      throw dependency_resolution_error(std::string(typeid(Interface).name()) +
                                        std::string(" not found"));
    }

    return std::static_pointer_cast<Interface>(bindings[typeID]());
  }

private:
  template <typename Interface> int getTypeID() {
    static int id = typeID()++;
    return id;
  }

  int& typeID() {
    static int typeID = 0;
    return typeID;
  }

  std::map<int, std::function<std::shared_ptr<void>()>> bindings;
};
}
