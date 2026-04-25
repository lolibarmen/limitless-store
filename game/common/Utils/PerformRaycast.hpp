#pragma once


Dictionary OuterWildsCharacter::perform_raycast(float max_distance) {
    // Получаем начало и направление луча из камеры
    Vector3 ray_origin = camera->get_global_position();
    Vector3 ray_direction = -camera->get_global_transform().basis.get_column(2); // forward
    Vector3 ray_end = ray_origin + ray_direction * max_distance;

    // Параметры запроса
    Ref<PhysicsRayQueryParameters3D> query = PhysicsRayQueryParameters3D::create(ray_origin, ray_end);
    query->set_collide_with_bodies(true);
    // query->set_exclude({ get_rid() }); // исключить самого персонажа, если нужно

    // Выполняем запрос
    PhysicsDirectSpaceState3D *space_state = get_world_3d()->get_direct_space_state();
    return space_state->intersect_ray(query);
}