#include "WorldCoordinator.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <SemanticWorld/SemanticWorld.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <WorldMesh/WorldMesh.hpp>
#include <SpatialStreaming/SpatialStreaming.hpp>
#include <PlanetShape/PlanetShape.hpp>

#include <vector>

using namespace godot;

void WorldCoordinator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_shapes", "shapes"), &WorldCoordinator::set_shapes);
    ClassDB::bind_method(D_METHOD("get_shapes"), &WorldCoordinator::get_shapes);
    ADD_PROPERTY(
        PropertyInfo(Variant::ARRAY, "shapes", PROPERTY_HINT_ARRAY_TYPE, "24/17:SemanticShape"),
        "set_shapes", "get_shapes"
    );
}

void WorldCoordinator::set_shapes(const TypedArray<Ref<SemanticShape>>& shapes) {
    if (_registering) {
        _shapes = shapes;
        return;
    }

    if (_exiting) {
        return;
    }

    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (sw) {
        for (int i = 0; i < _shapes.size(); ++i) {
            Ref<SemanticShape> old_shape = _shapes[i];
            if (old_shape.is_null() || old_shape->get_id() == 0) {
                continue;
            }

            bool still_present = false;
            for (int j = 0; j < shapes.size(); ++j) {
                Ref<SemanticShape> candidate = shapes[j];
                if (candidate.is_valid() && candidate.ptr() == old_shape.ptr()) {
                    still_present = true;
                    break;
                }
            }

            if (!still_present) {
                sw->unregister_shape_with_owned(old_shape->get_id());
            }
        }
    }

    _shapes = shapes;
    _register_shapes();
}

TypedArray<Ref<SemanticShape>> WorldCoordinator::get_shapes() const {
    return _shapes;
}

void WorldCoordinator::_attach_render_nodes() {
    auto attach = [this](Node* node) {
        if (!node) return;
        if (node->get_parent() == this) return;
        if (node->get_parent()) {
            node->get_parent()->remove_child(node);
        }
        add_child(node, false, Node::INTERNAL_MODE_FRONT);
    };

    attach(SpatialStreaming::get_singleton());
    attach(WorldMesh::get_singleton());
}

void WorldCoordinator::_detach_render_nodes() {
    auto detach = [this](Node* node) {
        if (node && node->get_parent() == this) {
            remove_child(node);
        }
    };

    detach(SpatialStreaming::get_singleton());
    detach(WorldMesh::get_singleton());
}

void WorldCoordinator::_enter_tree() {
    _exiting = false;
    _attach_render_nodes();
    _register_shapes();
}

void WorldCoordinator::_exit_tree() {
    if (_exiting) {
        return;
    }

    _exiting = true;

    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (sw) {
        std::vector<uint64_t> ids;
        ids.reserve(_shapes.size());
        for (int i = 0; i < _shapes.size(); ++i) {
            Ref<SemanticShape> shape = _shapes[i];
            if (shape.is_valid() && shape->get_id() != 0) {
                ids.push_back(shape->get_id());
            }
        }

        for (uint64_t id : ids) {
            sw->unregister_shape_with_owned(id);
        }
    }

    _detach_render_nodes();
    _exiting = false;
}

void WorldCoordinator::_register_shapes() {
    if (_exiting || !is_inside_tree()) {
        return;
    }

    SemanticWorld* sw = SemanticWorld::get_singleton();
    if (!sw) {
        return;
    }

    ShapeSpatialManager* mgr = ShapeSpatialManager::get_singleton();

    _registering = true;

    // Планеты регистрируются первыми: остальные фигуры привязываются к их октодеревьям.
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 0; i < _shapes.size(); ++i) {
            Ref<SemanticShape> shape = _shapes[i];
            if (shape.is_null() || shape->get_id() != 0) {
                continue;
            }

            const bool is_planet = Object::cast_to<PlanetShape>(shape.ptr()) != nullptr;
            if ((pass == 0) != is_planet) {
                continue;
            }

            sw->register_shape(shape);

            if (mgr && !mgr->get_octree(shape->get_id())) {
                UtilityFunctions::printerr(
                    "WorldCoordinator: фигура '", shape->get_shape_type(),
                    "' (id ", shape->get_id(),
                    ") не имеет октодерева. Проверьте on_register() и привязку planet."
                );
            }
        }
    }

    _registering = false;
}
