#include "gtest/gtest.h"

#include "diydi/diydi.h"
#include "diydi/graph.h"

class IG {};
class G : public IG {};
class IF {};
class F : public IF {
 public:
  F(std::shared_ptr<IG> g) {}
};
class IE {};
class E : public IE {};
class ID {};
class D : public ID {
 public:
  D(std::shared_ptr<IG> g) {}
};
class IC {};
class C : public IC {
 public:
  C(std::shared_ptr<IF> f) {}
};
class IB {};
class B : public IB {
 public:
  B(std::shared_ptr<ID> d, std::shared_ptr<IE> e) {}
};
class IA {};
class A : public IA {
 public:
  A(std::shared_ptr<IB> b, std::shared_ptr<IC> c) {}
};

TEST(Graph, test_save) {
  diydi::Injector injector;

  injector.bind<IA, A, IB, IC>();
  injector.bind<IB, B, ID, IE>();
  injector.bind<IC, C, IF>();
  injector.bind<ID, D, IG>();
  injector.bind<IE, E>();
  injector.bind<IG, G>();
  injector.bind<IF, F, IG>();

  diydi::Graph graph(injector);

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

  ASSERT_EQ(graph.generateDotFile(), expected);
}
