#include "gtest/gtest.h"

#include "diydi/diydi.h"

class IGreeter {
public:
  virtual std::string greet() = 0;
  virtual ~IGreeter() = default;
};

class IName {
public:
  virtual std::string name() = 0;
  virtual ~IName() = default;
};

class DefaultGreeter : public IGreeter {
public:
  DefaultGreeter() {}
  std::string greet() { return "hello, world"; }
};

class CustomizableGreeter : public IGreeter {
public:
  CustomizableGreeter(std::shared_ptr<IName> name) : name(name) {}
  std::string greet() { return "hello, " + name->name(); }

private:
  std::shared_ptr<IName> name;
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
  container.bind<IGreeter, CustomizableGreeter, IName>();

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

  container.bind<IName, UniverseName>(diydi::Scope::SINGLETON);

  ASSERT_EQ(container.getInstance<IName>(), container.getInstance<IName>());
}

TEST(DIYDI, test_duplicate_bind_calls) {
  diydi::Container container;

  container.bind<IName, UniverseName>();

  ASSERT_THROW((container.bind<IName, UniverseName>()),
               diydi::already_bound_error);
}

TEST(DIYDI, test_invalid_graph) {
  diydi::Container container;

  container.bind<IGreeter, CustomizableGreeter, IName>();

  ASSERT_THROW(container.getInstance<IGreeter>(),
               diydi::dependency_resolution_error);
}
