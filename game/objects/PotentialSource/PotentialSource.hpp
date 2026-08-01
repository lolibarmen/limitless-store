#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <Interfaces/Source.hpp>
#include <GameStructs/Cell.hpp>

namespace godot {

class PotentialSource : public Resource, public Source<CellData> {
    GDCLASS(PotentialSource, Resource)

    CellData generate(const Vector3i& world_pos) const override {
        return {0.0f};
    }

protected:
    static void _bind_methods();

public:
    PotentialSource() = default;
    ~PotentialSource() override = default;
};

} // namespace godot