#include "gtest/gtest.h"

#include "diydi/diydi.h"

class IName {
   public:
    virtual std::string name() = 0;
    virtual ~IName() = default;
};

class IGreeter {
   public:
    virtual std::string greet() = 0;
    virtual ~IGreeter() = default;
};

class IDecorativeGreeterFactory {
   public:
    virtual std::shared_ptr<IGreeter> create(std::string prefix, std::string suffix) = 0;
    virtual ~IDecorativeGreeterFactory() = default;
};

class DefaultGreeter : public IGreeter {
   public:
    INJECT(DefaultGreeter()) {}
    std::string greet() { return "hello, world"; }
};

class GenericGreeter : public IGreeter {
   public:
    INJECT(GenericGreeter(std::shared_ptr<IName> name)) : name(name) {}
    std::string greet() { return "hello, " + name->name(); }

   private:
    std::shared_ptr<IName> name;
};

class DecorativeGreeter : public IGreeter {
   public:
    using Inject = DecorativeGreeter(std::shared_ptr<IName>);

    DecorativeGreeter(std::shared_ptr<IName> name, std::string prefix, std::string suffix)
        : name(name), prefix(prefix), suffix(suffix) {}
    std::string greet() { return prefix + "hello, " + name->name() + suffix; }

   private:
    std::shared_ptr<IName> name;
    std::string prefix;
    std::string suffix;
};

class UniverseName : public IName {
   public:
    INJECT(UniverseName()) {}
    std::string name() { return "universe"; }
};

TEST(DIYDI, test_simple_bind_and_get) {
    diydi::Injector injector;

    injector.bind<IGreeter, DefaultGreeter>();

    std::shared_ptr<IGreeter> instance = injector.getInstance<IGreeter>();
    ASSERT_EQ(instance->greet(), "hello, world");
}

TEST(DIYDI, test_nested_bind_and_get) {
    diydi::Injector injector;

    injector.bind<IName, UniverseName>();
    injector.bind<IGreeter, GenericGreeter>();

    std::shared_ptr<IGreeter> instance = injector.getInstance<IGreeter>();
    ASSERT_EQ(instance->greet(), "hello, universe");
}

TEST(DIYDI, test_default_scope) {
    diydi::Injector injector;

    injector.bind<IName, UniverseName>();

    ASSERT_NE(injector.getInstance<IName>(), injector.getInstance<IName>());
}

TEST(DIYDI, test_singleton_scope) {
    diydi::Injector injector;

    injector.bindSingleton<IName, UniverseName>();

    ASSERT_EQ(injector.getInstance<IName>(), injector.getInstance<IName>());
}

TEST(DIYDI, test_configuration_injection) {
    diydi::Injector injector;

    injector.bind<IName, UniverseName>();
    injector.bind<IGreeter, DecorativeGreeter>("* ", "!");

    ASSERT_EQ(injector.getInstance<IGreeter>()->greet(), "* hello, universe!");
}

TEST(DIYDI, test_factory) {
    diydi::Injector injector;

    injector.bind<IName, UniverseName>();
    // clang-format off
  injector.bind<IDecorativeGreeterFactory,
                 diydi::Factory<IDecorativeGreeterFactory>
                    ::Implements<IGreeter, DecorativeGreeter>
                    ::Dependencies<IName>
                    ::Arguments<std::string, std::string>>();
    // clang-format on

    std::shared_ptr<IDecorativeGreeterFactory> greeterFactory =
        injector.getInstance<IDecorativeGreeterFactory>();
    ASSERT_EQ(greeterFactory->create("* ", "!")->greet(), "* hello, universe!");
}

TEST(DIYDI, test_duplicate_bind_calls) {
    diydi::Injector injector;

    injector.bind<IName, UniverseName>();

    ASSERT_THROW((injector.bind<IName, UniverseName>()), diydi::already_bound_error);
}

TEST(DIYDI, test_invalid_graph) {
    diydi::Injector injector;

    injector.bind<IGreeter, GenericGreeter>();

    ASSERT_THROW(injector.getInstance<IGreeter>(), diydi::dependency_resolution_error);
}

#define TYPE(if_name, impl_name, ...)          \
    class if_name {                            \
       public:                                 \
        virtual void call() = 0;               \
        virtual ~if_name() = default;          \
    };                                         \
    class impl_name : public if_name {         \
       public:                                 \
        using Inject = impl_name(__VA_ARGS__); \
        impl_name(__VA_ARGS__) {}              \
        void call() {}                         \
    };

TYPE(IG, G)
TYPE(IF, F, std::shared_ptr<IG>)
TYPE(IE, E)
TYPE(ID, D, std::shared_ptr<IG>)
TYPE(IC, C, std::shared_ptr<IF>)
TYPE(IB, B, std::shared_ptr<ID>, std::shared_ptr<IE>)
TYPE(IA, A, std::shared_ptr<IB>, std::shared_ptr<IC>)

TEST(DIYDI, test_dot_file) {
    diydi::Injector injector;

    injector.bind<IA, A>();
    injector.bind<IB, B>();
    injector.bind<IC, C>();
    injector.bind<ID, D>();
    injector.bind<IE, E>();
    injector.bind<IF, F>();
    injector.bind<IG, G>();

    std::string expected = 1 + R"(
digraph diydi {
    "A" -> {"B", "C"};
    "B" -> {"D", "E"};
    "C" -> {"F"};
    "D" -> {"G"};
    "E" -> {};
    "F" -> {"G"};
    "G" -> {};
})";

    ASSERT_EQ(injector.asDotFile(), expected);
}
