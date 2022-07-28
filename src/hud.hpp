#pragma once

#include <qpl/qpl.hpp>
#include "data.hpp"

struct hud {
	void draw(qsf::draw_object& draw) const {
		draw.draw(this->background);
		draw.draw(this->text);
		draw.draw(this->sprite);
	}

	void init() {
		this->text.set_font("gidugu");
		this->text.set_character_size(60u);
		this->text.set_position({ 30, 15 });

		this->sprite.set_texture(qsf::get_texture("leaf"));
		this->sprite.set_scale(1.0f / 10);
		
		this->animation.set_duration(2.0);
	}
	void reset() {
		this->text.set_color(qpl::rgb::white);
	}

	void set_dimension(qpl::vector2f dimension) {
		this->background.set_dimension(qpl::vec(dimension.x, data::hud_height));
		this->background.set_color(qpl::rgb(20, 20, 20));
		this->background.set_outline_thickness(5.0f);
		this->background.set_outline_color(qpl::rgb(150, 150, 150));
	}

	void update(const qsf::event_info& event) {
		this->animation.update(event.frame_time_f());
		if (this->animation.is_running()) {
			auto p = this->animation.get_curve_progress(1.5);
			auto c = this->interpolate_color.interpolated(qpl::rgb::white, p);
			this->text.set_color(c);
		}
	}
	void start_animation(qpl::rgb color) {
		this->interpolate_color = color;
		this->animation.reset_and_start();
	}

	void set_info(qpl::size amount) {
		this->amount = amount;
		this->text.set_string("You have ", amount);
		auto hitbox = this->text.get_visible_hitbox();
		auto pos = hitbox.middle_right().moved(40, 0);
		this->sprite.set_center(pos);
	}

	qsf::rectangle background;
	qsf::text text;
	qsf::sprite sprite;
	qpl::animation animation;
	qpl::rgb interpolate_color;
	qpl::size amount;
};