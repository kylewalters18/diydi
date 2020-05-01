#include "gtest/gtest.h"

#include "diydi/diydi.h"
#include "diydi/graph.h"

class IG {};
class G : public IG {
   public:
    INJECT(G()) {}
};

class IF {};
class F : public IF {
   public:
    INJECT(F(std::shared_ptr<IG> g)) {}
};

class IE {};
class E : public IE {
   public:
    INJECT(E()) {}
};

class ID {};
class D : public ID {
   public:
    INJECT(D(std::shared_ptr<IG> g)) {}
};

class IC {};
class C : public IC {
   public:
    INJECT(C(std::shared_ptr<IF> f)) {}
};

class IB {};
class B : public IB {
   public:
    INJECT(B(std::shared_ptr<ID> d, std::shared_ptr<IE> e)) {}
};

class IA {};
class A : public IA {
   public:
    INJECT(A(std::shared_ptr<IB> b, std::shared_ptr<IC> c)) {}
};

TEST(Graph, test_save) {
    diydi::Injector injector;

    injector.bind<IA, A>();
    injector.bind<IB, B>();
    injector.bind<IC, C>();
    injector.bind<ID, D>();
    injector.bind<IE, E>();
    injector.bind<IF, F>();
    injector.bind<IG, G>();

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
