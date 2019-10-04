#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <type_traits>
#include <typeinfo>

#ifndef DIYDI_DIYDI_H_
#define DIYDI_DIYDI_H_

namespace diydi {

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

template <typename FactoryInterface>
class Factory {
 public:
  template <typename Interface, typename Implementation>
  class Implements {
   public:
    template <typename... Deps>
    class Dependencies {
     public:
      template <typename... Args>
      class Arguments : public FactoryInterface {
       public:
        Arguments(std::shared_ptr<Deps>... deps)
            : factory([deps...](Args... args) -> std::shared_ptr<Interface> {
                static_assert(std::is_base_of<Interface, Implementation>::value,
                              "Implementation must inherit from Interface");

                return std::make_shared<Implementation>(deps..., args...);
              }) {}

        std::shared_ptr<Interface> create(Args... args) {
          return factory(args...);
        }

       private:
        std::function<std::shared_ptr<Interface>(Args...)> factory;
      };
    };
  };
};

class Injector {
 public:
  Injector() {}
  Injector(const Injector&) = delete;
  Injector& operator=(const Injector&) = delete;

  template <typename Interface, typename Implementation,
            typename... Dependencies, typename... Arguments>
  void bind(Arguments... args) {
    internalBind<Interface, Implementation, Dependencies...>(Scope::DEFAULT,
                                                             args...);
  }

  template <typename Interface, typename Implementation,
            typename... Dependencies, typename... Arguments>
  void bindSingleton(Arguments... args) {
    internalBind<Interface, Implementation, Dependencies...>(Scope::SINGLETON,
                                                             args...);
  }

  template <typename Interface>
  std::shared_ptr<Interface> getInstance() {
    int typeID = getTypeID<Interface>();
    if (!bindings.count(typeID)) {
      throw dependency_resolution_error(std::string(typeid(Interface).name()) +
                                        std::string(" not found"));
    }

    return std::static_pointer_cast<Interface>(bindings[typeID]());
  }

 private:
  enum Scope { DEFAULT, SINGLETON };

  template <typename Interface, typename Implementation,
            typename... Dependencies, typename... Arguments>
  void internalBind(Scope scope, Arguments... args) {
    static_assert(std::is_base_of<Interface, Implementation>::value,
                  "Implementation must inherit from Interface");

    int typeID = getTypeID<Interface>();
    if (bindings.count(typeID)) {
      throw already_bound_error(std::string(typeid(Interface).name()) +
                                std::string(" already bound"));
    }

    if (scope == Scope::DEFAULT) {
      bindings[typeID] = [this, args...]() {
        return std::make_shared<Implementation>(getInstance<Dependencies>()...,
                                                args...);
      };
    } else if (scope == Scope::SINGLETON) {
      bindings[typeID] = [this, args...]() {
        static std::shared_ptr<Implementation> instance =
            std::make_shared<Implementation>(getInstance<Dependencies>()...,
                                             args...);
        return instance;
      };
    }
  }

  template <typename Interface>
  int getTypeID() {
    static int id = typeID()++;
    return id;
  }

  int& typeID() {
    static int typeID = 0;
    return typeID;
  }

  std::map<int, std::function<std::shared_ptr<void>()>> bindings;
};
}  // namespace diydi

#endif  // DIYDI_DIYDI_H_
