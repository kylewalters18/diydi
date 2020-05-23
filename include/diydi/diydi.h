#ifndef DIYDI_DIYDI_H_
#define DIYDI_DIYDI_H_

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <type_traits>
#include <typeinfo>
#include <vector>

#include <cxxabi.h>

namespace diydi {
template <typename Annotation, typename Type>
class Annotated;
};

/**
 * Convenience macro used for automatically capturing the constructor
 * signature of bound types.
 *
 * Thanks to Google's fruit for the following macro tricks https://github.com/google/fruit/
 *
 * Example usage:
 *
 *  class GenericGreeter : public IGreeter {
 *   public:
 *      INJECT(GenericGreeter(std::shared_ptr<IName> name)) : name(name) {}
 *      std::string greet() { return "hello, " + name->name(); }
 *   private:
 *      std::shared_ptr<IName> name);
 *  };
 *
 *  int main() {
 *      Injector injector
 *      injector.bind<IGreeter, GenericGreeter>();
 *      std::shared_ptr<IGreeter> greeter = injector.getInstance<IGreeter>();
 *  }
 */
#define INJECT(Signature)                                              \
    using Inject = Signature;                                          \
                                                                       \
    template <typename Annotation, typename AnnotatedDeclarationParam> \
    using AnnotatedTypedef = AnnotatedDeclarationParam;                \
                                                                       \
    Signature

/**
 * Convenience macro used for automatically capturing annotated injections
 *
 * Example usage:
 *
 *  class MultiGreeter : public IGreeter {
 *   public:
 *      INJECT(MultiGreeter(ANNOTATED(Universe, std::shared_ptr<IName>) universe,
 *                          ANNOTATED(Galaxy, std::shared_ptr<IName>) galaxy))
 *          : universe(universe), galaxy(galaxy) {}
 *      std::string greet() { return "hello, " + universe->name() + " and " + galaxy->name(); }
 *
 *   private:
 *      std::shared_ptr<IName> universe;
 *      std::shared_ptr<IName> galaxy;
 *  };
 *
 *  int main() {
 *      struct Universe {};
 *      struct Galaxy {};
 *
 *      Injector injector
 *      injector.bind<Annotated<Universe, IName>, UniverseName>();
 *      injector.bind<Annotated<Galaxy, IName>, GalaxyName>();
 *      injector.bind<IGreeter, MultiGreeter>();
 *      std::shared_ptr<IGreeter> greeter = injector.getInstance<IGreeter>();
 *  }
 */
#define ANNOTATED(Annotation, ...) AnnotatedTypedef<Annotation, __VA_ARGS__>

template <typename Annotation, typename T>
using AnnotatedTypedef = diydi::Annotated<Annotation, T>;

namespace diydi {

template <typename Annotation, typename Type>
class Annotated : public Type {};

/**
 * Provides a generic way for getting the bound type from the INJECT
 * and ANNOTATED API.
 */
template <typename T>
struct remove_ptr;

/**
 * Specialization of remove_ptr for annotations
 */
template <typename A, typename T>
struct remove_ptr<Annotated<A, std::shared_ptr<T>>> {
    using type = Annotated<A, T>;
};

/**
 * Specialization of remove_ptr for std::shared_ptr
 */
template <typename T>
struct remove_ptr<std::shared_ptr<T>> {
    using type = T;
};

/**
 * Provides a generic way for getting the interface types between default
 * and annotated injection types.
 */
template <typename T>
struct get_if {
    using type = T;
};

/**
 * Specialization of get_if for annotations
 */
template <typename A, typename T>
struct get_if<Annotated<A, T>> {
    using type = T;
};

/**
 * Demangles the typename of T
 */
template <typename T>
static std::string demangle() {
    int status;
    char* demangledType = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
    std::string demangled(demangledType);
    free(demangledType);  // NOLINT(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
    return demangled;
}

/**
 * Excpetion type to indicate duplicate bind calls to the same interface
 */
class already_bound_error : public std::exception {
   public:
    explicit already_bound_error(const std::string& msg) : msg(msg.c_str()) {}
    const char* what() const noexcept override { return msg; }

   private:
    const char* msg;
};

/**
 * Excpetion type to indicate that a type was required but not bound
 */
class dependency_resolution_error : public std::exception {
   public:
    explicit dependency_resolution_error(const std::string& msg) : msg(msg.c_str()) {}
    const char* what() const noexcept override { return msg; }

   private:
    const char* msg;
};

/**
 * Factories are an extablished pattern for creating objects that require
 * dependencies and parameterization. This class can automate the boilerplate
 * required to create factories from simple interfaces.
 *
 * Example usage:
 *
 *   diydi::Factory<IDecorativeGreeterFactory>
 *        ::Implements<IGreeter, DecorativeGreeter>
 *        ::Dependencies<IName>
 *        ::Arguments<std::string, std::string>>()
 */
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
                    : factory([deps...](Args... args) -> std::shared_ptr<Interface> {
                          static_assert(std::is_base_of<Interface, Implementation>::value,
                                        "Implementation must inherit from Interface");

                          return std::make_shared<Implementation>(deps..., args...);
                      }) {}

                std::shared_ptr<Interface> create(Args... args) { return factory(args...); }

               private:
                std::function<std::shared_ptr<Interface>(Args...)> factory;
            };
        };
    };
};

/**
 * Wiring together objects is a tedious excercise when using dependency
 * injection. Injector provides a way to automate that by wiring interfaces
 * to implementations and a method to get an instance of a bound object. It
 * does this by doing a depth first search traversal in constructing the
 * object graph.
 */
class Injector {
   public:
    Injector() = default;
    ~Injector() = default;
    Injector(const Injector&) = delete;
    Injector& operator=(const Injector&) = delete;
    Injector(Injector&&) = default;
    Injector& operator=(Injector&&) = default;

    /**
     * Binds an interface to an implementation using the default scope of
     * creating a new object each time an implementation of it is needed.
     * It optionally takes a list of arguments that will be supplied by the
     * injector framework.
     */
    template <typename Interface, typename Implementation, typename... Arguments>
    void bind(Arguments... args) {
        internalBind<Interface, Implementation>(Scope::DEFAULT, args...);
    }

    /**
     * Binds an interface to an implementation using the singleton scope. This
     * allows you to reuse the object each time that an implementation is
     * needed. It optionally takes a list of arguments that will be supplied by
     * the injector framework.
     */
    template <typename Interface, typename Implementation, typename... Arguments>
    void bindSingleton(Arguments... args) {
        internalBind<Interface, Implementation>(Scope::SINGLETON, args...);
    }

    /**
     * Returns an instance of the requested interface. This method builds the
     * object graph by a depth first search algorithm.
     */
    template <typename Interface>
    std::shared_ptr<Interface> getInstance() const {
        int typeID = getTypeID<Interface>();

        if (!bindings.count(typeID)) {
            throw dependency_resolution_error(std::string(typeid(Interface).name()) +
                                              std::string(" not found"));
        }

        return std::static_pointer_cast<Interface>(bindings.at(typeID)());
    }

    /**
     * Returns a string in the dot file format that represents the graph as an
     * adjaceny list
     *
     * See https://en.wikipedia.org/wiki/DOT_(graph_description_language)
     */
    std::string asDotFile() {
        std::stringstream buffer;

        buffer << "digraph diydi {";

        for (const auto& node : adjacencyList) {
            int nodeID = node.first;

            buffer << "\n    \"";
            buffer << adjacencyList[nodeID]->implementation();
            buffer << "\" -> {";

            std::vector<int> adjacent = adjacencyList[nodeID]->adjacent(*this);
            for (size_t i = 0; i < adjacent.size(); i++) {
                buffer << "\"";
                buffer << adjacencyList[adjacent[i]]->implementation();
                buffer << (i == adjacent.size() - 1 ? "\"" : "\", ");
            }

            buffer << "};";
        }
        buffer << "\n}";

        return buffer.str();
    }

   private:
    enum Scope { DEFAULT, SINGLETON };

    template <typename Interface, typename Implementation, typename... Arguments>
    void internalBind(Scope scope, Arguments... args) {
        static_assert(std::is_base_of<typename get_if<Interface>::type, Implementation>::value,
                      "Implementation must inherit from Interface");

        int typeID = getTypeID<Interface>();
        if (bindings.count(typeID)) {
            throw already_bound_error(std::string(typeid(Interface).name()) +
                                      std::string(" already bound"));
        }

        auto node = std::make_shared<Node<Interface, typename Implementation::Inject>>();
        adjacencyList[getTypeID<Interface>()] = node;

        if (scope == Scope::DEFAULT) {
            bindings[typeID] = [this, node, args...]() { return node->create(*this, args...); };
        } else if (scope == Scope::SINGLETON) {
            bindings[typeID] = [this, node, args...]() {
                static std::shared_ptr<Implementation> instance = node->create(*this, args...);
                return instance;
            };
        }
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

    class INode {
       public:
        virtual std::string interface() = 0;
        virtual std::string implementation() = 0;
        virtual std::vector<int> adjacent(const Injector& injector) = 0;
    };

    template <typename Interface = void, typename Signature = void()>
    class Node;

    template <typename Interface, typename Implementation, typename... Dependencies>
    class Node<Interface, Implementation(Dependencies...)> : public INode {
       public:
        template <typename... Arguments>
        std::shared_ptr<Implementation> create(const Injector& injector,
                                               Arguments... arguments) const {
            return std::make_shared<Implementation>(
                injector.getInstance<typename remove_ptr<Dependencies>::type>()..., arguments...);
        }

        std::string interface() override { return demangle<Interface>(); }
        std::string implementation() override { return demangle<Implementation>(); }
        std::vector<int> adjacent(const Injector& injector) override {
            return std::vector<int>(
                {injector.getTypeID<typename remove_ptr<Dependencies>::type>()...});
        }
    };

    std::map<int, std::function<std::shared_ptr<void>()>> bindings;
    std::map<int, std::shared_ptr<INode>> adjacencyList;
};
}  // namespace diydi

#endif  // DIYDI_DIYDI_H_
