#include "WorldShapeNode.hpp"
#include <WorldCoordinator/WorldCoordinator.hpp>
#include <PlanetShape/PlanetShape.hpp>
#include <TreeShape/TreeShape.hpp>
#include <ShapeSpatialManager/ShapeSpatialManager.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

using namespace godot;

void WorldShapeNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_shape", "shape"), &WorldShapeNode::set_shape);
    ClassDB::bind_method(D_METHOD("get_shape"), &WorldShapeNode::get_shape);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "SemanticShape"), "set_shape", "get_shape");

    ClassDB::bind_method(D_METHOD("_on_shape_changed"), &WorldShapeNode::_on_shape_changed);
}

void WorldShapeNode::set_shape(const Ref<SemanticShape>& p_shape) {
    if (_shape == p_shape) {
        return;
    }

    if (_shape.is_valid() && _shape->is_connected("changed", callable_mp(this, &WorldShapeNode::_on_shape_changed))) {
        _shape->disconnect("changed", callable_mp(this, &WorldShapeNode::_on_shape_changed));
    }

    _shape = p_shape;

    if (_shape.is_valid()) {
        _shape->connect("changed", callable_mp(this, &WorldShapeNode::_on_shape_changed));
        _sync_from_shape();
    }

    _request_coordinator_refresh();
}

Ref<SemanticShape> WorldShapeNode::get_shape() const {
    return _shape;
}

Vector3 WorldShapeNode::_get_shape_space_origin() const {
    if (_shape.is_null()) {
        return Vector3();
    }
    if (Object::cast_to<PlanetShape>(_shape.ptr())) {
        return Vector3();
    }
    TreeShape *tree = Object::cast_to<TreeShape>(_shape.ptr());
    if (tree) {
        Ref<PlanetShape> planet = tree->get_planet();
        if (planet.is_valid()) {
            return planet->get_position();
        }
    }
    if (_shape->get_id() != 0) {
        ShapeSpatialManager *mgr = ShapeSpatialManager::get_singleton();
        if (mgr) {
            ChunkOctree *octree = mgr->get_octree(_shape->get_id());
            if (octree) {
                return octree->get_position();
            }
        }
    }
    return Vector3();
}

void WorldShapeNode::_sync_to_shape() {
    if (_syncing || _shape.is_null()) {
        return;
    }
    _syncing = true;
    _shape->set_position(get_position() - _get_shape_space_origin());
    _syncing = false;
}

void WorldShapeNode::_sync_from_shape() {
    if (_syncing || _shape.is_null()) {
        return;
    }
    _syncing = true;
    set_position(_get_shape_space_origin() + _shape->get_position());
    _syncing = false;
}

void WorldShapeNode::_on_shape_changed() {
    _sync_from_shape();
}

void WorldShapeNode::_request_coordinator_refresh() {
    WorldCoordinator* wc = Object::cast_to<WorldCoordinator>(get_parent());
    if (wc) {
        wc->refresh_shapes();
    }
}

void WorldShapeNode::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
            set_notify_local_transform(true);
            _sync_from_shape();
            _request_coordinator_refresh();
        } break;
        case NOTIFICATION_EXIT_TREE: {
            _request_coordinator_refresh();
        } break;
        case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
            _sync_to_shape();
            PlanetShape *planet = Object::cast_to<PlanetShape>(_shape.ptr());
            if (planet) {
                Node *parent = get_parent();
                if (parent) {
                    for (int i = 0; i < parent->get_child_count(); ++i) {
                        WorldShapeNode *wn = Object::cast_to<WorldShapeNode>(parent->get_child(i));
                        if (!wn || wn == this) {
                            continue;
                        }
                        Ref<SemanticShape> child_shape = wn->get_shape();
                        TreeShape *tree = Object::cast_to<TreeShape>(child_shape.ptr());
                        if (tree && tree->get_planet().is_valid() && tree->get_planet().ptr() == planet) {
                            wn->_sync_from_shape();
                        }
                    }
                }
            }
        } break;
        default:
            break;
    }
}
