#pragma once

#include <qpl/qpl.hpp>
#include "data.hpp"

struct race_floor {

	void create(qpl::size snail_count, double goal) {
		this->snail_count = snail_count;
		this->goal = goal;
	}
	void init() {
		this->start_line.set_color(qpl::rgb(50, 150, 50));
		this->finish_line.set_color(qpl::rgb(150, 50, 50));

		this->tile_map.set_texture(qsf::get_texture("grass"), 32);
	}
	void apply_animation() {
		auto p = this->animation.get_curve_progress();
		auto a = qpl::u8_cast(p * 255);

		this->start_line.set_multiplied_alpha(a);
		this->finish_line.set_multiplied_alpha(a);
		for (auto& i : this->seperations) {
			i.set_multiplied_alpha(a);
		}
		for (auto& i : this->tile_map.vertices) {
			i.color = qpl::rgba::white.with_alpha(a);
		}
	}
	void reset() {
		this->animation.reset();
	}
	void set_dimension(qpl::vector2f dimension) {

		auto diff = data::snail_delta_y * ((this->snail_count - 1) / 2.0f);
		auto pos = qpl::vec(data::snail_start_x + data::snail_offset, data::hud_height + ((dimension.y - data::hud_height) / 2) - diff);
		pos.y += this->snail_count * data::snail_delta_y - 50;

		auto dim = qpl::vec(this->goal * 10'000 + data::snail_width, 5);

		auto line_height = this->snail_count * data::snail_delta_y;
		constexpr auto line_width = 5;

		this->start_line.set_position(pos - qpl::vec(0, line_height));
		this->start_line.set_dimension(qpl::vec(line_width, line_height));

		this->finish_line.set_position(pos.moved(dim.x, 0) - qpl::vec(line_width, line_height));
		this->finish_line.set_dimension(qpl::vec(line_width, line_height));

		constexpr auto scale = 4;
		constexpr auto resolution = 32;

		auto x = (dim.x - 1) / (resolution * scale) + 1;
		auto y = (line_height - 1) / (resolution * scale) + 1;

		this->tile_map.position = pos - qpl::vec(0, line_height);
		this->tile_map.scale = qpl::vec(scale, scale);

		this->tile_map.create(0u, qpl::vec(dim.x, line_height));
	}

	void fade_in() {
		this->animation.reset_and_start();
	}
	bool finished_fading_in() const {
		return this->animation.just_finished_no_reverse();
	}
	void fade_out() {
		this->animation.reset_and_start_reverse();
	}

	void update(const qsf::event_info& event) {
		this->animation.update(event.frame_time_f());
		if (this->animation.is_running()) {
			this->apply_animation();
		}
	}

	void draw(qsf::draw_object& draw) const {
		draw.draw(this->tile_map);
		draw.draw(this->start_line);
		draw.draw(this->finish_line);
	}

	std::vector<qsf::rectangle> seperations;
	qsf::rectangle start_line;
	qsf::rectangle finish_line;
	qpl::size snail_count = 0u;
	qsf::small_tile_map tile_map;
	qpl::animation animation;
	double goal = 0.0;
};