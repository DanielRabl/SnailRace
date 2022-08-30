#pragma once

#include <qpl/qpl.hpp>

struct bet_feedback {

	void init() {
		this->text.set_font("gidugu");
		this->text.set_character_size(150);

		this->text_background.set_dimension({ 600, 150 });
		this->text_background.set_outline_thickness(2.0);
		this->text_background.set_color(qpl::rgb(40, 40, 40));
		this->text_background.set_outline_color(qpl::rgb::grey_shade(220));

		this->shadow.set_color(qpl::rgba(10, 10, 20, 200));
	}

	void make_position() {
		this->text.set_center(data::get_center_after_hud(this->dimension));
		this->text_background.set_hitbox(this->text.get_visible_hitbox().increased(50));
	}

	void set_dimension(qpl::vector2f dimension) {
		this->dimension = dimension;
		this->shadow.set_dimension(dimension);
		this->make_position();
	}
	void set_text(std::string text) {
		this->text.set_string(text);
		this->make_position();
	}
	void set_color(qpl::rgb color) {
		this->text_background.set_outline_color(color);
		this->text.set_color(color);
	}

	void fade_in() {
		this->animation.reset_and_start();
	}
	void reset() {
		this->text.set_multiplied_alpha(0u);
		this->text_background.set_multiplied_alpha(0u);
		this->shadow.set_multiplied_alpha(0u);
		this->animation.reset();
	}

	void update(const qsf::event_info& event) {
		this->animation.update(event.frame_time_f());
		if (this->animation.is_running()) {
			auto p = this->animation.get_curve_progress();
			auto a = qpl::u8_cast(p * 255);
			this->text.set_multiplied_alpha(a);
			this->text_background.set_multiplied_alpha(a);
			this->shadow.set_multiplied_alpha(a);
		}

		if (this->animation.just_finished_no_reverse()) {
			qpl::start_timed_task(0.5, [&]() {
				this->animation.go_backwards();
				});
		}
		this->shadow.set_dimension(dimension);
	}
	void draw(qsf::draw_object& draw) const {
		draw.draw(this->shadow);
		draw.draw(this->text_background);
		draw.draw(this->text);
	}

	qsf::rectangle shadow;
	qsf::text text;
	qpl::vector2f dimension;
	qpl::animation animation;
	qsf::smooth_rectangle text_background;
};