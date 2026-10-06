#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/material.hpp>

namespace godot {

class MaterialGenerator : public Resource {
    GDCLASS(MaterialGenerator, Resource)

protected:
    static void _bind_methods();

public:
    MaterialGenerator() {};
    virtual ~MaterialGenerator() {};

    virtual Ref<Material> generate(uint64_t shape_id, uint64_t chunk_id) const;
};

} // namespace godot