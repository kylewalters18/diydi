#ifndef DIYDI_DIYDI_H_
#define DIYDI_DIYDI_H_

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include <cxxabi.h>

namespace diydi {

#define INJECT(Signature)     \
    using Inject = Signature; \
    Signature

class already_bound_error : public std::exception {
   public:
    explicit already_bound_error(const std::string& msg) : msg(msg.c_str()) {}
    const char* what() const noexcept override { return msg; }

   private:
    const char* msg;
};

class dependency_resolution_error : public std::exception {
   public:
    explicit dependency_resolution_error(const std::string& msg)
        : msg(msg.c_str()) {}
    const char* what() const noexcept override { return msg; }

   private:
    const char* msg;
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
                using Inject = Arguments(std::shared_ptr<Deps>... deps);

                explicit Arguments(std::shared_ptr<Deps>... deps)
                    : factory([deps...](
                                  Args... args) -> std::shared_ptr<Interface> {
                          static_assert(
                              std::is_base_of<Interface, Implementation>::value,
                              "Implementation must inherit from Interface");

                          return std::make_shared<Implementation>(deps...,
                                                                  args...);
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

struct Node {
    std::vector<int> adjacent;
    std::string interfaceType;
    std::string concreteType;
};

class Injector {
   public:
    Injector() = default;
    ~Injector() = default;
    Injector(const Injector&) = delete;
    Injector& operator=(const Injector&) = delete;
    Injector(Injector&&) = default;
    Injector& operator=(Injector&&) = default;

    template <typename Interface,
              typename Implementation,
              typename... Arguments>
    void bind(Arguments... args) {
        internalBind<Interface, Implementation>(Scope::DEFAULT, args...);
    }

    template <typename Interface,
              typename Implementation,
              typename... Arguments>
    void bindSingleton(Arguments... args) {
        internalBind<Interface, Implementation>(Scope::SINGLETON, args...);
    }

    template <typename Interface>
    std::shared_ptr<Interface> getInstance() const {
        int typeID = getTypeID<Interface>();

        if (!bindings.count(typeID)) {
            throw dependency_resolution_error(
                std::string(typeid(Interface).name()) +
                std::string(" not found"));
        }

        return std::static_pointer_cast<Interface>(bindings.at(typeID)());
    }

    std::map<int, Node> getGraph() const { return graph; }

   private:
    enum Scope { DEFAULT, SINGLETON };

    template <typename Interface,
              typename Implementation,
              typename... Arguments>
    void internalBind(Scope scope, Arguments... args) {
        static_assert(std::is_base_of<Interface, Implementation>::value,
                      "Implementation must inherit from Interface");

        int typeID = getTypeID<Interface>();
        if (bindings.count(typeID)) {
            throw already_bound_error(std::string(typeid(Interface).name()) +
                                      std::string(" already bound"));
        }

        Constructor<typename Implementation::Inject> constructor(*this);

        graph[getTypeID<Interface>()] = {constructor.adjacent(),
                                         demangle<Interface>(),
                                         demangle<Implementation>()};

        if (scope == Scope::DEFAULT) {
            bindings[typeID] = [this, constructor, args...]() {
                return constructor.create(args...);
            };
        } else if (scope == Scope::SINGLETON) {
            bindings[typeID] = [this, constructor, args...]() {
                static std::shared_ptr<Implementation> instance =
                    constructor.create(args...);
                return instance;
            };
        }
    }

    template <typename T>
    static std::string demangle() {
        int status;
        char* demangledType =
            abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
        std::string demangled(demangledType);
        free(demangledType);  // NOLINT(cppcoreguidelines-owning-memory,
                              // cppcoreguidelines-no-malloc)
        return demangled;
    }

    template <typename Interface>
    int getTypeID() const {
        static int id = typeID()++;
        return id;
    }

    int& typeID() const {
        static int typeID = 0;
        return typeID;
    }

    template <typename T>
    struct Pointer;

    template <typename T>
    struct Pointer<std::shared_ptr<T>> {
        using type = T;
    };

    template <typename Signature = void()>
    class Constructor;

    template <typename Return, typename... Dependencies>
    class Constructor<Return(Dependencies...)> {
       public:
        explicit Constructor(const Injector& injector) : injector(injector) {}

        template <typename... Arguments>
        std::shared_ptr<Return> create(Arguments... arguments) const {
            return std::make_shared<Return>(
                injector.getInstance<typename Pointer<Dependencies>::type>()...,
                arguments...);
        }

        std::vector<int> adjacent() {
            return std::vector<int>(
                {injector
                     .getTypeID<typename Pointer<Dependencies>::type>()...});
        }

       private:
        const Injector& injector;
    };

    std::map<int, std::function<std::shared_ptr<void>()>> bindings;
    std::map<int, Node> graph;
};
}  // namespace diydi

#endif  // DIYDI_DIYDI_H_
