#pragma once

#include <qpl/qpl.hpp>
#include "data.hpp"

struct race_announcement {

	void draw(qsf::draw_object& draw) const {
		draw.draw(this->text_rectangle);
		draw.draw(this->text);
	}

	void set_colors() {
		this->text.set_color(qpl::rgb(255, 150, 200));
		this->text_rectangle.set_outline_color(qpl::rgb(255, 200, 230));
		this->text_rectangle.set_color(qpl::rgb::transparent);
	}
	void init() {
		this->text.set_font("gidugu");
		this->text.set_character_size(150u);

		this->animation.set_duration(1.0);

		this->text_rectangle.set_outline_thickness(4.0f);
		this->text_rectangle.set_slope_dimension(40);
	}
	void update(const qsf::event_info& event) {
		this->animation.update(event.frame_time_f());
		if (this->animation.is_running()) {
			if (this->animation.just_finished_no_reverse()) {
				qpl::start_timed_task(1.0, [&]() {
					this->animation.go_backwards();
					});
			}

			auto p = this->animation.get_curve_progress(1.0);

			auto a = qpl::u8_cast(255 * p);
			this->text.set_multiplied_alpha(a);
			this->text_rectangle.set_multiplied_alpha(a);
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
	void set_info(std::string text) {
		this->text.set_string(text);
		this->text.set_center(data::get_center_after_hud(this->dimension));
		this->text_rectangle.set_hitbox(this->text.get_visible_hitbox().increased(50));
	}

	qsf::text text;
	qsf::smooth_rectangle text_rectangle;
	qpl::vector2f dimension;
	qpl::animation animation;
};