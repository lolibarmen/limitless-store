#pragma once
#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>

#include <PlayerMovement/PlayerMovement.hpp>
#include <PlayerCamera/PlayerCamera.hpp>
#include <PlayerInteraction/PlayerInteraction.hpp>
#include <ToolManager/ToolManager.hpp>

namespace godot {

class Player : public CharacterBody3D {
    GDCLASS(Player, CharacterBody3D)

private:
    CollisionShape3D* collision_shape = nullptr;

    Ref<PlayerMovement> movement;
    Ref<PlayerCamera> camera_ctl;
    Ref<PlayerInteraction> interaction;
    ToolManager* tool_mgr = nullptr;

protected:
    static void _bind_methods() {}

public:
    Player() = default;

    void _ready() override;
    void _physics_process(double delta) override;
    void _process(double delta) override;
    void _input(const Ref<InputEvent>& event) override;
};

}