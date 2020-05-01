#include <cxxabi.h>
#include <fstream>

#ifndef DIYDI_GRAPH_H_
#define DIYDI_GRAPH_H_

namespace diydi {

class Injector;

class Graph {
   public:
    Graph(const Injector& injector) : injector(injector) {}

    void save(std::string filename) {
        std::ofstream outfile(filename);

        outfile << generateDotFile();

        outfile.close();
    }

    std::string generateDotFile() {
        std::map<int, Node> graph = injector.getGraph();

        std::stringstream buffer;

        buffer << "digraph diydi {";

        for (const auto& node : graph) {
            int nodeID = node.first;

            buffer << "\n    \"";
            buffer << graph[nodeID].concreteType;
            buffer << "\" -> {";

            std::vector<int> adjacent = graph[nodeID].adjacent;
            for (size_t i = 0; i < adjacent.size(); i++) {
                buffer << "\"";
                buffer << graph[adjacent[i]].concreteType;
                buffer << (i == adjacent.size() - 1 ? "\"" : "\", ");
            }

            buffer << "};";
        }
        buffer << "\n}";

        return buffer.str();
    }

   private:
    const Injector& injector;
};
}  // namespace diydi

#endif  // DIYDI_GRAPH_H_
