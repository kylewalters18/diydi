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
  virtual std::shared_ptr<IGreeter> create(std::string prefix,
                                           std::string suffix) = 0;
  virtual ~IDecorativeGreeterFactory() = default;
};

class DefaultGreeter : public IGreeter {
public:
  DefaultGreeter() {}
  std::string greet() { return "hello, world"; }
};

class GenericGreeter : public IGreeter {
public:
  GenericGreeter(std::shared_ptr<IName> name) : name(name) {}
  std::string greet() { return "hello, " + name->name(); }

private:
  std::shared_ptr<IName> name;
};

class DecorativeGreeter : public IGreeter {
public:
  DecorativeGreeter(std::shared_ptr<IName> name,
                    std::string prefix,
                    std::string suffix)
      : name(name), prefix(prefix), suffix(suffix) {}
  std::string greet() { return prefix + "hello, " + name->name() + suffix; }

private:
  std::shared_ptr<IName> name;
  std::string prefix;
  std::string suffix;
};

class UniverseName : public IName {
public:
  std::string name() { return "universe"; }
};

TEST(DIYDI, test_simple_bind_and_get) {
  diydi::Container container;

  container.bind<IGreeter, DefaultGreeter>();

  std::shared_ptr<IGreeter> instance = container.getInstance<IGreeter>();
  ASSERT_EQ(instance->greet(), "hello, world");
}

TEST(DIYDI, test_nested_bind_and_get) {
  diydi::Container container;

  container.bind<IName, UniverseName>();
  container.bind<IGreeter, GenericGreeter, IName>();

  std::shared_ptr<IGreeter> instance = container.getInstance<IGreeter>();
  ASSERT_EQ(instance->greet(), "hello, universe");
}

TEST(DIYDI, test_default_scope) {
  diydi::Container container;

  container.bind<IName, UniverseName>();

  ASSERT_NE(container.getInstance<IName>(), container.getInstance<IName>());
}

TEST(DIYDI, test_singleton_scope) {
  diydi::Container container;

  container.bindSingleton<IName, UniverseName>();

  ASSERT_EQ(container.getInstance<IName>(), container.getInstance<IName>());
}

TEST(DIYDI, test_configuration_injection) {
  diydi::Container container;

  container.bind<IName, UniverseName>();
  container.bind<IGreeter, DecorativeGreeter, IName>("* ", "!");

  ASSERT_EQ(container.getInstance<IGreeter>()->greet(), "* hello, universe!");
}

TEST(DIYDI, test_factory) {
  diydi::Container container;

  container.bind<IName, UniverseName>();
  // clang-format off
  container.bind<IDecorativeGreeterFactory,
                 diydi::Factory<IDecorativeGreeterFactory>
                    ::Implements<IGreeter, DecorativeGreeter>
                    ::Dependencies<IName>
                    ::Arguments<std::string, std::string>,
                 IName>();
  // clang-format on

  std::shared_ptr<IDecorativeGreeterFactory> greeterFactory =
      container.getInstance<IDecorativeGreeterFactory>();
  ASSERT_EQ(greeterFactory->create("* ", "!")->greet(), "* hello, universe!");
}

TEST(DIYDI, test_duplicate_bind_calls) {
  diydi::Container container;

  container.bind<IName, UniverseName>();

  ASSERT_THROW((container.bind<IName, UniverseName>()),
               diydi::already_bound_error);
}

TEST(DIYDI, test_invalid_graph) {
  diydi::Container container;

  container.bind<IGreeter, GenericGreeter, IName>();

  ASSERT_THROW(container.getInstance<IGreeter>(),
               diydi::dependency_resolution_error);
}
