#include <benchmark/benchmark.h>

#include "diydi/diydi.h"

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

TYPE(IA, A)
TYPE(IB, B, std::shared_ptr<IA>)
TYPE(IC, C, std::shared_ptr<IA>, std::shared_ptr<IB>)

static void BM_Bind(benchmark::State& state) {
    while (state.KeepRunning()) {
        diydi::Injector injector;
        injector.bind<IA, A>();
        injector.bind<IB, B>();
        injector.bind<IC, C>();
    }
}
// Register the function as a benchmark
BENCHMARK(BM_Bind);

// Define another benchmark
static void BM_GetInstance(benchmark::State& state) {
    diydi::Injector injector;
    injector.bind<IA, A>();
    injector.bind<IB, B>();
    injector.bind<IC, C>();

    for (auto _ : state) {
        std::shared_ptr<IC> c = injector.getInstance<IC>();
    }
}
// Register the function as a benchmark
BENCHMARK(BM_GetInstance);

BENCHMARK_MAIN();
