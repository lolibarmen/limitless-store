#pragma once
#include <Tool/Tool.hpp>

namespace godot {

class SuperDigger : public Tool {
    GDCLASS(SuperDigger, Tool)

protected:
    static void _bind_methods() {}

public:
    virtual bool can_use_on(const Dictionary &raycast_result) const override;
    virtual bool can_use_alt_on(const Dictionary &raycast_result) const override;
    virtual void use(const Dictionary &raycast_result) override;
    virtual void use_alt(const Dictionary &raycast_result) override;

    void dig(const Dictionary &raycast_result, float radius, float delta);
};

} // namespace godot