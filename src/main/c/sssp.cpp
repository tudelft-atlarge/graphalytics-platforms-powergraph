#include <graphlab.hpp>
#include <stdint.h>

#include "algorithms.hpp"
#include "utils.hpp"

using namespace std;

typedef double vertex_data_type;
typedef double edge_data_type;
typedef graphlab::empty gather_type;
typedef min_reducer<vertex_data_type> msg_type;
typedef graphlab::distributed_graph<vertex_data_type, edge_data_type> graph_type;

static void init_vertex(graph_type::vertex_type &vertex) {
    vertex.data() = numeric_limits<vertex_data_type>::max();
}

static bool global_directed;


class single_source_shortest_path :
    public graphlab::ivertex_program<graph_type, gather_type, msg_type>,
    public graphlab::IS_POD_TYPE {

    msg_type last_msg;
    bool changed;

    public:
        void init(icontext_type& context, const vertex_type& vertex, const msg_type& msg) {
            last_msg = msg;
        }

        edge_dir_type gather_edges(icontext_type& context, const vertex_type& vertex) const {
            return graphlab::NO_EDGES;
        }

        void apply(icontext_type& context, vertex_type& vertex, const gather_type &total) {
            if (last_msg.get() < vertex.data()) {
                vertex.data() = last_msg.get();
                changed = true;
            } else {
                changed = false;
            }
        }

        edge_dir_type scatter_edges(icontext_type& context, const vertex_type& vertex) const {
            return changed
                    ? (global_directed
                            ? graphlab::OUT_EDGES
                            : graphlab::ALL_EDGES)
                    : graphlab::NO_EDGES;
        }

        void scatter(icontext_type& context, const vertex_type& vertex, edge_type& edge) const {
            const vertex_type& other = edge.target().id() == vertex.id() ? edge.source() : edge.target();
            vertex_data_type new_dist = vertex.data() + edge.data();

            if (other.data() > new_dist) {
                context.signal(other, msg_type(new_dist));
            }
        }
};

bool edge_data_parser(const string &line, double &data) {
    const char *start = line.c_str();
    char *end;
    data = strtod(start, &end);
    return start < end;
}

void run_sssp(context_t &ctx, bool directed, graphlab::vertex_id_type source) {
    timer_start();

    // process parameters
    global_directed = directed;

    // load graph
    timer_next("load graph");
    graph_type graph(ctx.dc);
    load_graph_properties(graph, ctx, default_parser<vertex_data_type>, edge_data_parser);
    graph.finalize();
    graph.transform_vertices(init_vertex);

    // start engine
    timer_next("initialize engine");
    graphlab::omni_engine<single_source_shortest_path> engine(ctx.dc, graph, "synchronous", ctx.clopts);
    engine.signal(source, msg_type(0.0));

    // run algorithm
    timer_next("run algorithm");
    engine.start();

    // print output
    if (ctx.output_stream) {
    	timer_next("print output");
        vector<pair<graphlab::vertex_id_type, vertex_data_type> > data;
        collect_vertex_data(graph, data);

        for (size_t i = 0; i < data.size(); i++) {
            vertex_data_type d = data[i].second;

            // If the distance is the max value for vertex_data_type
            // then the vertex is not connected to the source vertex.
            // According to specs, the output should be +inf
            if (d == numeric_limits<vertex_data_type>::max()) {
                d = numeric_limits<vertex_data_type>::infinity();
            }

            (*ctx.output_stream) << data[i].first << " " << d << endl;
        }
    }

    timer_end();
}