#pragma once

#include <qpl/qpl.hpp>

struct start_button {

	void init() {
		this->button.set_text_font("gidugu");
		this->button.set_text_character_size(50);
		this->button.set_text_string("PLACE BET");
		this->button.set_background_color(qpl::rgb(20, 20, 40, 150));
		this->button.set_background_outline_thickness(2.0);
		this->button.set_background_outline_color(qpl::rgb::white);
		this->button.set_dimension({ 250, 70 });
	}

	void set_dimension(qpl::vector2f dimension) {
		auto dim = this->button.get_dimension();
		this->button.set_position(dimension - dim - qpl::vec(30, 30));
		this->button.centerize_text();
	}
	void reset() {
		this->visible = false;
		this->animation.reset();
		this->button.set_multiplied_alpha(0);
	}
	void update(const qsf::event_info& event) {
		event.update(this->button);
	}
	void update_animation(const qsf::event_info& event) {

		this->animation.update(event.frame_time_f());
		if (this->animation.is_running()) {
			auto p = this->animation.get_curve_progress();
			auto a = qpl::u8_cast(255 * p);
			this->button.set_multiplied_alpha(a);
		}
	}
	void draw(qsf::draw_object& draw) const {
		draw.draw(this->button);
	}

	qsf::smooth_button button;
	qpl::animation animation;
	bool visible = false;
};