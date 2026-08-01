#pragma once
#include <Tool/Tool.hpp>

namespace godot {

class PotentialEditor : public Tool {
    GDCLASS(PotentialEditor, Tool)

protected:
    static void _bind_methods() {}

public:
    virtual bool can_use_on(const Dictionary &raycast_result) const override { return true; }
    virtual bool can_use_alt_on(const Dictionary &raycast_result) const override { return true; }
    virtual void use(const Dictionary &raycast_result) override;
    virtual void use_alt(const Dictionary &raycast_result) override {};
};

} // namespace godot