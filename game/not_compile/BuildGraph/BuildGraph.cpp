#include "BuildGraph.hpp"
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/classes/object.hpp>

using namespace godot;

BuildGraph *BuildGraph::singleton = nullptr;

void BuildGraph::_bind_methods() {
    ClassDB::bind_method(D_METHOD("connect_nodes", "a", "b", "a_socket_id", "b_socket_id"),
                          &BuildGraph::connect_nodes, DEFVAL(String()), DEFVAL(String()));
    ClassDB::bind_method(D_METHOD("disconnect_nodes", "a", "b"), &BuildGraph::disconnect_nodes);
    ClassDB::bind_method(D_METHOD("remove_node", "node"), &BuildGraph::remove_node);
    ClassDB::bind_method(D_METHOD("are_connected", "a", "b"), &BuildGraph::are_connected);
    ClassDB::bind_method(D_METHOD("get_connected", "node"), &BuildGraph::get_connected);
    ClassDB::bind_method(D_METHOD("clear"), &BuildGraph::clear);

    ADD_SIGNAL(MethodInfo("connection_added",
        PropertyInfo(Variant::OBJECT, "a"), PropertyInfo(Variant::OBJECT, "b")));
    ADD_SIGNAL(MethodInfo("connection_removed",
        PropertyInfo(Variant::OBJECT, "a"), PropertyInfo(Variant::OBJECT, "b")));
}

BuildGraph::BuildGraph() {
    ERR_FAIL_COND_MSG(singleton != nullptr, "BuildGraph singleton уже создан.");
    singleton = this;
}

BuildGraph::~BuildGraph() {
    if (singleton == this) singleton = nullptr;
}

BuildGraph *BuildGraph::get_singleton() {
    return singleton;
}

void BuildGraph::_watch(Node3D *p_node) {
    ObjectID id(p_node->get_instance_id());
    if (watched.has(id)) return;
    watched.insert(id);
    // Когда узел уходит из дерева (в т.ч. непосредственно перед queue_free) — чистим граф
    p_node->connect("tree_exiting", callable_mp(this, &BuildGraph::remove_node).bind(p_node));
}

void BuildGraph::connect_nodes(Node3D *a, Node3D *b,
                                const String &a_socket_id, const String &b_socket_id) {
    ERR_FAIL_NULL(a);
    ERR_FAIL_NULL(b);
    if (a == b) return;

    ObjectID id_a(a->get_instance_id());
    ObjectID id_b(b->get_instance_id());

    Vector<Edge> &edges_a = adjacency[id_a];
    for (int i = 0; i < edges_a.size(); ++i)
        if (edges_a[i].other == id_b) return; // уже соединены

    edges_a.push_back(Edge{ id_b, a_socket_id, b_socket_id });
    adjacency[id_b].push_back(Edge{ id_a, b_socket_id, a_socket_id });

    _watch(a);
    _watch(b);

    emit_signal("connection_added", a, b);
}

void BuildGraph::disconnect_nodes(Node3D *a, Node3D *b) {
    ERR_FAIL_NULL(a);
    ERR_FAIL_NULL(b);
    ObjectID id_a(a->get_instance_id());
    ObjectID id_b(b->get_instance_id());

    if (Vector<Edge> *edges = adjacency.getptr(id_a)) {
        for (int i = 0; i < edges->size(); ++i)
            if ((*edges)[i].other == id_b) { edges->remove_at(i); break; }
    }
    if (Vector<Edge> *edges = adjacency.getptr(id_b)) {
        for (int i = 0; i < edges->size(); ++i)
            if ((*edges)[i].other == id_a) { edges->remove_at(i); break; }
    }

    emit_signal("connection_removed", a, b);
}

void BuildGraph::remove_node(Node3D *node) {
    if (!node) return;
    ObjectID id(node->get_instance_id());

    if (Vector<Edge> *edges = adjacency.getptr(id)) {
        for (const Edge &e : *edges) {
            if (Vector<Edge> *other_edges = adjacency.getptr(e.other)) {
                for (int i = 0; i < other_edges->size(); ++i)
                    if ((*other_edges)[i].other == id) { other_edges->remove_at(i); break; }
            }
        }
        adjacency.erase(id);
    }
    watched.erase(id);
}

bool BuildGraph::are_connected(Node3D *a, Node3D *b) const {
    if (!a || !b) return false;
    ObjectID id_a(a->get_instance_id());
    const Vector<Edge> *edges = adjacency.getptr(id_a);
    if (!edges) return false;
    ObjectID id_b(b->get_instance_id());
    for (const Edge &e : *edges)
        if (e.other == id_b) return true;
    return false;
}

TypedArray<Node3D> BuildGraph::get_connected(Node3D *node) const {
    TypedArray<Node3D> result;
    if (!node) return result;
    ObjectID id(node->get_instance_id());
    const Vector<Edge> *edges = adjacency.getptr(id);
    if (!edges) return result;
    for (const Edge &e : *edges) {
        Object *obj = ObjectDB::get_instance(e.other);
        if (Node3D *n = Object::cast_to<Node3D>(obj))
            result.push_back(n);
    }
    return result;
}

void BuildGraph::clear() {
    adjacency.clear();
    watched.clear();
}