#include "gtest/gtest.h"

#include "diydi/diydi.h"
#include "diydi/graph.h"

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
