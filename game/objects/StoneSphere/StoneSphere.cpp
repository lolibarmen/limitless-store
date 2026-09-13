#include "StoneSphere.hpp"

namespace godot {

void StoneSphere::_bind_methods() {

}

StoneSphere::StoneSphere() {
    _generator.instantiate();
}

Ref<MeshGenerator> StoneSphere::get_mesh_generator() const {
    return _generator;
}

} // namespace godot