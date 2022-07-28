#pragma once

#include <qpl/qpl.hpp>
#include "data.hpp"

struct win_announcement {

	void draw(qsf::draw_object& draw) const {
		draw.draw(this->text_rectangle);
		draw.draw(this->text);
		draw.draw(this->leaf);
	}

	void init() {
		this->text.set_font("gidugu");
		this->text.set_character_size(150u);
		this->text.set_color(qpl::rgb(255, 150, 200));

		this->leaf.set_texture(qsf::get_texture("leaf"));
		this->leaf.set_scale(0.2f);

		this->animation.set_duration(1.0);

		this->text_rectangle.set_color(qpl::rgb::transparent);
		this->text_rectangle.set_outline_thickness(4.0f);
		this->text_rectangle.set_slope_dimension(40);
		this->text_rectangle.set_outline_color(qpl::rgb(200, 100, 100));
	}

	void apply_animation() {

		auto p = this->animation.get_curve_progress(1.0);
		auto a = qpl::u8_cast(255 * p);

		this->text.set_multiplied_alpha(a);
		this->text_rectangle.set_multiplied_alpha(a);
		this->leaf.set_multiplied_alpha(a);
	}
	void update(const qsf::event_info& event) {
		this->animation.update(event.frame_time_f());
		if (this->animation.is_running()) {
			if (this->animation.just_finished_no_reverse()) {
				qpl::start_timed_task(1.0, [&]() {
					this->animation.go_backwards();
					});
			}
			this->apply_animation();
		}
	}
	void start_animation() {
		this->animation.reset_and_start();
	}
	bool animation_finished() const {
		return this->animation.just_finished_reverse();
	}
	void set_dimension(qpl::vector2f dimension) {
		this->dimension = dimension;
		this->set_info(this->text.get_string());
		this->text_rectangle.set_center(data::get_center_after_hud(this->dimension));
	}
	void set_color(qpl::rgb color) {
		this->text.set_color(color);
		this->text_rectangle.set_outline_color(color);
	}
	void set_info(std::string text) {

		this->text.set_string(text);

		auto shift = this->leaf.get_hitbox().dimension.x + 20;

		this->text.set_center(data::get_center_after_hud(this->dimension) - qpl::vec(shift / 2, 0));
		auto hitbox = this->text.get_visible_hitbox();
		this->leaf.set_position(hitbox.middle_right() + qpl::vec(20, 0));
		this->leaf.centerize_y();
		hitbox.dimension.x += shift;
		this->text_rectangle.set_hitbox(hitbox.increased(50));
	}

	qsf::text text;
	qsf::smooth_rectangle text_rectangle;
	qpl::vector2f dimension;
	qpl::animation animation;
	qsf::sprite leaf;
};