# diydi

[![Build Status](https://travis-ci.org/kylewalters18/diydi.svg?branch=master)](https://travis-ci.org/kylewalters18/diydi)

A micro framework for dependency injection in C++ loosely inspired by
[Guice](https://github.com/google/guice)

### Bindings

  Bindings are used to configure how the injector will build your graph of
  objects

  Suppose we have a simple application with two interfaces and implementations
  for each.

  ```cpp
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

  class WorldName : public IName {
   public:
      INJECT(WorldName()) = default;
      std::string name() { return "world"; }
  };

  class GenericGreeter : public IGreeter {
   public:
      // Alternatively, define a typedef that mimics the constructor to use
      // for injection
      //
      // using Inject = GenericGreeter(std::shared_ptr<IName>);

      INJECT(GenericGreeter(std::shared_ptr<IName> name)) : name(name) {}
      std::string greet() { return "hello, " + name->name(); }

   private:
      std::shared_ptr<IName> name;
  };
  ```

  Then to wire up the implementations to their interfaces

  ```cpp
  Injector injector;
  injector.bind<IName, WorldName>();
  injector.bind<IGreeter, GenericGreeter>();

  std::shared_ptr<IGreeter> greeter = injector.getInstance<IGreeter>();
  ```

### Annotated Injections

  Suppose we have a class that requires multiple dependencies that implement
  the same interface. We can use annotated injections to disambiguate the two

  ```cpp
  struct First {};
  struct Second {};

  class MultiGreeter : public IGreeter {
   public:
      INJECT(MultiGreeter(ANNOTATED(First, std::shared_ptr<IName>) first,
                          ANNOTATED(Second, std::shared_ptr<IName>) second))
          : first(first), second(second) {}
      std::string greet() { return "hello, " + first->name() + " and " + second->name(); }

   private:
      std::shared_ptr<IName> first;
      std::shared_ptr<IName> second;
   };

  int main() {
      diydi::Injector injector;

      injector.bind<diydi::Annotated<First, IName>, UniverseName>();
      injector.bind<diydi::Annotated<Second, IName>, GalaxyName>();
      injector.bind<IGreeter, MultiGreeter>();

      injector.getInstance<IGreeter>()->greet();
  }
  ```

### Scopes

  Scopes are used to control the lifetime of your objects. The injector
  produces a new instance per request by default. You can configure the
  injector to reuse objects for the lifetime of your application with the
  singleton scope.

  ```cpp
  Injector injector;
  injector.bindSingleton<IGreeter, DefaultGreeter>();
  ```

### Factories

  Diydi can automate a lot of the boilerplate that comes with writing factory
  code

  Suppose we have a class that requires a dependency and caller provided
  parameters

  ```cpp
  class DecorativeGreeter : public IGreeter {
   public:
      INJECT(DecorativeGreeter(std::shared_ptr<IName> name, std::string prefix, std::string suffix))
          : name(name), prefix(prefix), suffix(suffix) {}

      std::string greet() { return prefix + "hello, " + name->name() + suffix; }

   private:
      std::shared_ptr<IName> name;
      std::string prefix;
      std::string suffix;
  };
  ```

  Define an interface with a single create method where the arguments match the
  caller provided parameters

  ```cpp
  class IDecorativeGreeterFactory {
   public:
      virtual std::shared_ptr<IGreeter> create(std::string prefix,
                                               std::string suffix) = 0;
      virtual ~IDecorativeGreeterFactory() = default;
  };
  ```

  Then use the Factory class to create an implementation and bind it to your
  interface

  ```cpp
  Injector injector;
  injector.bind<IName, UniverseName>();
  injector.bind<IDecorativeGreeterFactory,
                Factory<IDecorativeGreeterFactory>
                    ::Implements<IGreeter, DecorativeGreeter>
                    ::Dependencies<IName>
                    ::Arguments<std::string, std::string>>();
  ```

### Injecting instances

  You can also inject your own instances of objects. This may be useful in
  situations where a parameter is a part of your applications configuration.

  ```cpp
  Injector injector;
  injector.bind<IGreeter, DecorativeGreeter>("** ", "!");
  ```

### Graphing

  You can generate a graph of your application in the dot file format

  ```cpp
  #include "diydi/graph.h"

  Graph graph(injector);
  graph.save("demo.dot");
  ```

  This will produce a file called `demo.dot`

  ```
  digraph diydi {
      "A" -> {"B", "C"};
      "B" -> {"D", "E"};
      "C" -> {"F"};
      "D" -> {"G"};
      "E" -> {};
      "F" -> {"G"};
      "G" -> {};
  }
  ```

  The resulting file can be processed by various programs but we'll use the dot
  program to generate a png

  ```sh
  dot -T png demo.dot -o demo.png
  ```

  ![](https://user-images.githubusercontent.com/9455230/66232973-68947f00-e6a7-11e9-9db4-f31bf86bcfae.png)


### Tests

  The examples above came from the tests. You can run them natively with

  ```sh
  make build test
  ```

### Development

  A list of commonly used tasks while working on this project can be found by running

  ```sh
  make
  ```
