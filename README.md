# diydi

[![Build Status](https://travis-ci.org/kylewalters18/diydi.svg?branch=master)](https://travis-ci.org/kylewalters18/diydi)

A micro framework for dependency injection in C++ loosely inspired by
[Guice](https://github.com/google/guice)

### Bindings

  Bindings are used to configure how the injector will build your graph of objects

  Suppose we have an interface and an implementation of that interface

  ```cpp
  class IGreeter {
   public:
      virtual std::string greet() = 0;
      virtual ~IGreeter() = default;
  };

  class DefaultGreeter : public IGreeter {
   public:
      DefaultGreeter() {}
      std::string greet() { return "hello, world"; }
  };
  ```

  Then to wire up DefaultGreeter to IGreeter

  ```cpp
  Injector injector;
  injector.bind<IGreeter, DefaultGreeter>();

  std::shared_ptr<IGreeter> greeter = injector.getInstance<IGreeter>();
  ```

### Dependencies

  The injector needs to be configured with a list of a concrete class's
  dependencies in the order they appear in the constructor

  ```cpp
  class GenericGreeter : public IGreeter {
   public:
       GenericGreeter(std::shared_ptr<IName> name) : name(name) {}
       std::string greet() { return "hello, " + name->name(); }

   private:
       std::shared_ptr<IName> name;
   };
   ```

   Then to wire up GenericGreeter to IGreeter

   ```cpp
   Injector injector;
   injector.bind<IGreeter, GenericGreeter, IName>();
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
      DecorativeGreeter(std::shared_ptr<IName> name, std::string prefix,
                        std::string suffix)
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
                    ::Arguments<std::string, std::string>,
                IName>();
  ```

### Injecting instances

  You can also inject your own instances of objects. This may be useful in
  situations where a parameter is a part of your applications configuration.

  ```cpp
  Injector injector;
  injector.bind<IGreeter, DecorativeGreeter, IName>("** ", "!");
  ```

### Graphing

  You can also generate a graph of your application in the dot file format

  ```cpp
  #include "graph.h"

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
  dot -T png test.dot -o test.png
  ```

  <img src='https://g.gravizo.com/svg?
    digraph diydi {
        "A" -> {"B", "C"};
        "B" -> {"D", "E"};
        "C" -> {"F"};
        "D" -> {"G"};
        "E" -> {};
        "F" -> {"G"};
        "G" -> {};
    }
  '/>


### Tests

  The examples above came from the tests. You can run them natively with

  ```sh
  make .build .test
  ```

  Or if you prefer using the docker environment

  ```sh
  make
  ```
