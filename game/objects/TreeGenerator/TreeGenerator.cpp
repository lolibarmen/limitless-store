#include "TreeGenerator.hpp"
#include <TreeBlock/TreeBlock.hpp>
#include <MeshMarker/MeshMarker.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

void TreeGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_block_scene", "scene"), &TreeGenerator::set_block_scene);
	ClassDB::bind_method(D_METHOD("get_block_scene"), &TreeGenerator::get_block_scene);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "block_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_block_scene", "get_block_scene");

	ClassDB::bind_method(D_METHOD("set_num_blocks", "num"), &TreeGenerator::set_num_blocks);
	ClassDB::bind_method(D_METHOD("get_num_blocks"), &TreeGenerator::get_num_blocks);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "num_blocks", PROPERTY_HINT_RANGE, "1,50,1"), "set_num_blocks", "get_num_blocks");

	ClassDB::bind_method(D_METHOD("set_base_radius", "radius"), &TreeGenerator::set_base_radius);
	ClassDB::bind_method(D_METHOD("get_base_radius"), &TreeGenerator::get_base_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "base_radius", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_base_radius", "get_base_radius");

	ClassDB::bind_method(D_METHOD("set_block_height", "height"), &TreeGenerator::set_block_height);
	ClassDB::bind_method(D_METHOD("get_block_height"), &TreeGenerator::get_block_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "block_height", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_block_height", "get_block_height");

	ClassDB::bind_method(D_METHOD("set_taper", "taper"), &TreeGenerator::set_taper);
	ClassDB::bind_method(D_METHOD("get_taper"), &TreeGenerator::get_taper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "taper", PROPERTY_HINT_RANGE, "0.0,2.0,0.01"), "set_taper", "get_taper");

	ClassDB::bind_method(D_METHOD("set_bend_x", "bend"), &TreeGenerator::set_bend_x);
	ClassDB::bind_method(D_METHOD("get_bend_x"), &TreeGenerator::get_bend_x);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bend_x", PROPERTY_HINT_RANGE, "-1.0,1.0,0.01"), "set_bend_x", "get_bend_x");

	ClassDB::bind_method(D_METHOD("set_bend_z", "bend"), &TreeGenerator::set_bend_z);
	ClassDB::bind_method(D_METHOD("get_bend_z"), &TreeGenerator::get_bend_z);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bend_z", PROPERTY_HINT_RANGE, "-1.0,1.0,0.01"), "set_bend_z", "get_bend_z");

	ClassDB::bind_method(D_METHOD("generate"), &TreeGenerator::generate);
}

TreeGenerator::TreeGenerator() {
}

void TreeGenerator::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		generate();
	}
}

void TreeGenerator::set_block_scene(const Ref<PackedScene> &p_scene) {
	block_scene = p_scene;
}
Ref<PackedScene> TreeGenerator::get_block_scene() const {
	return block_scene;
}

void TreeGenerator::set_num_blocks(int p_num) {
	if (p_num != num_blocks) { num_blocks = p_num; generate(); }
}
int TreeGenerator::get_num_blocks() const { return num_blocks; }

void TreeGenerator::set_base_radius(real_t p_radius) {
	if (p_radius != base_radius) { base_radius = p_radius; generate(); }
}
real_t TreeGenerator::get_base_radius() const { return base_radius; }

void TreeGenerator::set_block_height(real_t p_height) {
	if (p_height != block_height) { block_height = p_height; generate(); }
}
real_t TreeGenerator::get_block_height() const { return block_height; }

void TreeGenerator::set_taper(real_t p_taper) {
	if (p_taper != taper) { taper = p_taper; generate(); }
}
real_t TreeGenerator::get_taper() const { return taper; }

void TreeGenerator::set_bend_x(real_t p_bend) {
	if (p_bend != bend_x) { bend_x = p_bend; generate(); }
}
real_t TreeGenerator::get_bend_x() const { return bend_x; }

void TreeGenerator::set_bend_z(real_t p_bend) {
	if (p_bend != bend_z) { bend_z = p_bend; generate(); }
}
real_t TreeGenerator::get_bend_z() const { return bend_z; }

void TreeGenerator::generate() {
	// 1. Очистка старых блоков
	for (int i = get_child_count() - 1; i >= 0; i--) {
		Node* child = get_child(i);
		if (Object::cast_to<TreeBlock>(child)) {
			remove_child(child);
			child->queue_free();
		}
	}

	// 2. Генерация с накоплением изгиба и смещения маркеров
	real_t current_radius = base_radius;
	Vector3 current_offset; // Накопленное смещение верха ствола

	for (int i = 0; i < num_blocks; ++i) {
		// Инстанцируем блок из сцены или создаём пустой
		TreeBlock* block = nullptr;
		if (block_scene.is_valid()) {
			block = Object::cast_to<TreeBlock>(block_scene->instantiate());
		}
		if (!block) {
			block = memnew(TreeBlock);
		}
		
		block->set_name("TreeBlock_" + itos(i));
		block->set_radius(current_radius);
		block->set_height(block_height);
		
		// Позиционируем весь блок по вертикали
		block->set_position(Vector3(0, i * block_height, 0) + current_offset);
		
		add_child(block);
		
		if (Engine::get_singleton()->is_editor_hint() && get_tree() && get_tree()->get_edited_scene_root()) {
			block->set_owner(get_tree()->get_edited_scene_root());
		}

		// 3. Вычисляем смещение для изгиба
		real_t bend_factor = (real_t)i / MAX(1, num_blocks - 1);
		Vector3 bend_offset = Vector3(bend_x * bend_factor, 0, bend_z * bend_factor) * block_height;
		
		// Обновляем накопленное смещение для следующего блока
		current_offset += bend_offset;

		// 4. Применяем сужение и изгиб к маркерам
		for (int c = 0; c < block->get_child_count(); c++) {
			MeshMarker* marker = Object::cast_to<MeshMarker>(block->get_child(c));
			if (!marker) continue;

			String name = marker->get_name();
			Vector3 original_pos = marker->get_position();

			// Определяем, верхний это маркер или нижний
			bool is_top = (original_pos.y > 0.0);

			// КЛЮЧЕВОЕ ИСПРАВЛЕНИЕ: 
			// Нижние маркеры имеют радиус current_radius
			// Верхние маркеры имеют радиус current_radius - taper (чтобы совпасть с нижними следующего блока)
			real_t marker_radius = is_top ? (current_radius - taper) : current_radius;
			marker_radius = MAX(0.001, marker_radius);

			// Масштабируем по X/Z относительно исходного радиуса из сцены
			Vector3 horizontal = Vector3(original_pos.x, 0, original_pos.z);
			real_t scale_factor = marker_radius / MAX(0.001, base_radius);
			horizontal *= scale_factor;

			Vector3 new_pos = Vector3(horizontal.x, original_pos.y, horizontal.z);

			// Верхние маркеры дополнительно смещаем для изгиба
			if (is_top) {
				new_pos += bend_offset;
			}

			marker->set_position(new_pos);
		}

		// 5. Принудительно пересобираем деформированный меш
		block->rebuild_deformed_mesh();

		// Уменьшаем радиус для следующей итерации
		current_radius -= taper;
		if (current_radius < 0.1) current_radius = 0.1;
	}
}