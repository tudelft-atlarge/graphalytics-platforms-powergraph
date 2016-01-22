#include <graphlab.hpp>
#include <boost/unordered_map.hpp>

#include "algorithms.hpp"
#include "utils.hpp"

using namespace std;

typedef int label_type;
typedef label_type vertex_data_type;
typedef graphlab::empty edge_data_type;
typedef histogram<label_type> gather_type;
typedef graphlab::distributed_graph<vertex_data_type, edge_data_type> graph_type;

void init_vertex(graph_type::vertex_type &vertex) {
    vertex.data() = vertex.id();
}

label_type most_common(const gather_type& total) {
    typedef gather_type::map_type map_type;
    const map_type m = total.get();

    label_type best_label = 0;
    size_t best_freq = 0;

    for (map_type::const_iterator it = m.begin(); it != m.end(); it++) {
        label_type label = it->first;
        size_t freq = it->second;

        if (freq > best_freq || (freq == best_freq && label < best_label)) {
            best_label = label;
            best_freq = freq;
        }
    }

    return best_label;
}

class label_propagation :
    public graphlab::ivertex_program<graph_type, gather_type>,
    public graphlab::IS_POD_TYPE {

    bool changed;

    public:
        edge_dir_type gather_edges(icontext_type& context, const vertex_type& vertex) const {
            return graphlab::ALL_EDGES;
        }

        gather_type gather(icontext_type& context, const vertex_type& vertex, edge_type& edge) const {
            const vertex_type& other = edge.source().id() == vertex.id() ? edge.target() : edge.source();
            return gather_type(other.data());
        }

        void apply(icontext_type& context, vertex_type& vertex, const gather_type &total) {
            vertex_data_type new_label = most_common(total);

            if (new_label != vertex.data()) {
                vertex.data() = new_label;
                changed = true;
            } else {
                changed = false;
            }
        }

        edge_dir_type scatter_edges(icontext_type& context, const vertex_type& vertex) const {
            return changed ? graphlab::ALL_EDGES : graphlab::NO_EDGES;
        }

        void scatter(icontext_type& context, const vertex_type& vertex, edge_type& edge) const {
            const vertex_type& other = edge.source().id() == vertex.id() ? edge.target() : edge.source();
            context.signal(other);
        }
};



void run_cd(context_t &ctx, int max_iter) {

    // process parameters
    ctx.clopts.engine_args.set_option("max_iterations", max_iter);

    // load graph
    graph_type graph(ctx.dc);
    load_graph(graph, ctx);
    graph.finalize();
    graph.transform_vertices(init_vertex);

    // run engine
    graphlab::omni_engine<label_propagation> engine(ctx.dc, graph, "synchronous", ctx.clopts);
    engine.signal_all();
    engine.start();

    // print output
    const float runtime = engine.elapsed_seconds();
    ctx.dc.cerr() << "finished in " << runtime << " sec" << endl;

    if (ctx.output_stream) {
        vector<pair<graphlab::vertex_id_type, vertex_data_type> > data;
        collect_vertex_data(graph, data);

        for (size_t i = 0; i < data.size(); i++) {
            (*ctx.output_stream) << data[i].first << " " << data[i].second << endl;
        }
    }

}
