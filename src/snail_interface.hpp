#pragma once

#include <qpl/qpl.hpp>
#include "player.hpp"

struct snail_interface {
	struct bet_type {
		qsf::sprite snail;
		qsf::sprite snail_border;
		qsf::sprite leaf;
		qsf::slider<qpl::u32> slider;
		qsf::text slider_text;
		qsf::text description_text;
		qsf::text winner_text;
		qsf::smooth_rectangle description_rectangle;
		qpl::animation winner_animation;
		qpl::rgb color;

		void draw(qsf::draw_object& draw) const {
			draw.draw(this->snail_border);
			draw.draw(this->snail);
			draw.draw(this->slider);
			draw.draw(this->slider_text);
			draw.draw(this->description_rectangle);
			draw.draw(this->description_text);
			draw.draw(this->leaf);
			draw.draw(this->winner_text);
		}

		void fade_winner_in(qpl::size winner_pos) {
			this->winner_text.set_string(qpl::to_string(winner_pos, '#'));

			switch (winner_pos) {
			case 1u:
				this->winner_text.set_color(qpl::rgb(255, 215, 0));
				break;
			case 2u:
				this->winner_text.set_color(qpl::rgb(169, 169, 200));
				break;

			case 3u:
				this->winner_text.set_color(qpl::rgb(205, 127, 50));
				break;
			default:
				this->winner_text.set_color(qpl::rgb(100, 100, 100));
			}
			this->winner_animation.reset_and_start();
		}

		void update_winner(const qsf::event_info& event) {
			this->winner_animation.update(event.frame_time_f());
			if (this->winner_animation.is_running()) {
				auto p = this->winner_animation.get_curve_progress();
				auto a = qpl::u8_cast(p * 255);
				this->winner_text.set_multiplied_alpha(a);
			}
		}

		void update(const qsf::event_info& event) {
			event.update(this->slider);
		}
	};

	void reset(const player& player) {
		for (auto& i : this->bet_types) {
			i.slider.set_range(0u, player.money, 0u);
			i.snail_border.set_color(i.color.darkened(0.8));
		}
		this->race_finished = false;
	}

	void create(const std::vector<snail_info>& snails, const player& player, double goal) {
		this->goal = goal;

		this->bet_types.resize(snails.size());
		for (qpl::size i = 0u; i < snails.size(); ++i) {
			auto scale = 0.3;

			this->bet_types[i].color = snails[i].color;
			this->bet_types[i].snail.set_texture(qsf::get_texture("snail"));
			this->bet_types[i].snail.set_scale(scale);

			this->bet_types[i].winner_text.set_font("gidugu");
			this->bet_types[i].winner_text.set_character_size(150);
			this->bet_types[i].winner_text.set_string("TEST");


			this->bet_types[i].leaf.set_texture(qsf::get_texture("leaf"));
			this->bet_types[i].leaf.set_scale(0.1);

			this->bet_types[i].snail_border.set_texture(qsf::get_texture("snail_border"));
			this->bet_types[i].snail_border.set_scale(scale);

			this->bet_types[i].snail.set_color(snails[i].color);

			this->bet_types[i].slider_text.set_font("gidugu");
			this->bet_types[i].slider_text.set_character_size(50);
			this->bet_types[i].slider_text.set_string("Einsatz: 0");


			this->bet_types[i].description_text.set_font("anonymous");
			this->bet_types[i].description_text.set_character_size(23);
			this->bet_types[i].description_text.set_string(snails[i].string());
			this->bet_types[i].description_text.set_line_spacing(0.9f);
			this->bet_types[i].description_text.set_color(qpl::rgb::white);

			this->bet_types[i].slider.set_dimensions({ 300, 40 }, { 30, 40 });
			this->bet_types[i].slider.set_hover_increase(1);

			this->bet_types[i].description_rectangle.set_color(qpl::rgb(20, 20, 20));
			this->bet_types[i].description_rectangle.set_outline_thickness(3.0f);
			this->bet_types[i].description_rectangle.set_outline_color(snails[i].color.with_alpha(50));
		}

		this->reset(player);
		this->seperations.resize(snails.size() - 1);
		for (auto& i : this->seperations) {
			i.set_color(qpl::rgb(200, 200, 200));
		}

		this->reset_visibility();
	}

	void reset_visibility() {
		for (auto& i : this->bet_types) {
			i.slider.set_alpha(0);
			i.snail.set_multiplied_alpha(0);
			i.snail_border.set_multiplied_alpha(0);
			i.slider_text.set_multiplied_alpha(0);
			i.description_rectangle.set_multiplied_alpha(0);
			i.description_text.set_multiplied_alpha(0);
			i.leaf.set_multiplied_alpha(0);
			i.winner_text.set_multiplied_alpha(0);
		}
		for (auto& i : this->seperations) {
			i.set_multiplied_alpha(0);
		}
		this->race_visible = false;
	}

	void set_positions(qpl::vector2f dimension) {

		auto diff = data::snail_delta_y * ((this->bet_types.size() - 1) / 2.0f);
		auto start = qpl::vec(data::snail_start_x, data::hud_height + ((dimension.y - data::hud_height) / 2) - diff);
		for (qpl::size i = 0u; i < this->bet_types.size(); ++i) {
			auto pos = start + qpl::vec(0, data::snail_delta_y * i);

			this->bet_types[i].snail.set_position(pos);
			this->bet_types[i].snail.centerize_y();
			this->bet_types[i].snail_border.set_position(pos);
			this->bet_types[i].snail_border.centerize_y();
			this->bet_types[i].slider.set_center(pos + qpl::vec(350, 0));

			this->bet_types[i].slider_text.set_center(pos + qpl::vec(200 + 400, 0));

			this->bet_types[i].slider_text.set_position(pos + qpl::vec(200 + 400, 0));
			this->bet_types[i].slider_text.centerize_y();

			this->bet_types[i].leaf.set_center(this->bet_types[i].slider_text.get_visible_hitbox().middle_right().moved(this->leaf_shift, 0));

			this->bet_types[i].description_text.set_center(pos - qpl::vec(170, 0));

			auto h = this->bet_types[i].description_text.get_visible_hitbox();
			this->bet_types[i].description_rectangle.set_hitbox(h.increased(10));

			if (i < this->seperations.size()) {
				this->seperations[i].set_position(qpl::vec(50, pos.y + data::snail_delta_y / 2));
				this->seperations[i].set_dimension(qpl::vec(dimension.x - 100, 1));
			}

			pos = qpl::vec(data::snail_start_x + 50 + data::snail_offset, data::hud_height + ((dimension.y - data::hud_height) / 2) - diff);
			pos.y += i * data::snail_delta_y;
			pos.x += this->goal * 10'000 + data::snail_width;

			this->bet_types[i].winner_text.set_position(pos);
			this->bet_types[i].winner_text.centerize_y();
		}
	}

	void apply_progress(const std::vector<snail_info>& snails) {
		for (qpl::size i = 0u; i < snails.size(); ++i) {
			this->bet_types[i].snail.set_position_x(snails[i].get_position());
			this->bet_types[i].snail_border.set_position_x(snails[i].get_position());
		}
	}

	void fade_in() {
		this->animation.reset_and_start();
	}
	void fade_out() {
		this->animation.reset_and_start_reverse();
	}
	bool finished_fading_out() const {
		return this->animation.just_finished_reverse();
	}

	void update(const qsf::event_info& event) {

		this->slider_changed = false;

		for (auto& i : this->bet_types) {
			i.update_winner(event);
		}

		this->animation.update(event.frame_time_f());
		if (this->animation.is_running()) {
			auto p = this->animation.get_curve_progress();
			auto a = qpl::u8_cast(255 * p);
			for (auto& i : this->bet_types) {

				if (this->race_finished) {
					i.snail.set_multiplied_alpha(a);
					i.snail_border.set_multiplied_alpha(a);
					i.winner_text.set_multiplied_alpha(a);
				}
				else {
					if (!this->race_visible) {
						i.snail.set_multiplied_alpha(a);
						i.snail_border.set_multiplied_alpha(a);
					}
					i.description_text.set_multiplied_alpha(a);
					i.description_rectangle.set_multiplied_alpha(a);
					i.slider.set_alpha(a);
					i.slider_text.set_multiplied_alpha(a);
					i.leaf.set_multiplied_alpha(a);

					for (auto& i : this->seperations) {
						i.set_multiplied_alpha(a);
					}
				}
			}
		}

		if (!this->animation.is_running() && this->animation.get_progress() == 0.0) {
			return;
		}
		event.update(this->bet_types);

		qpl::size index = qpl::size_max;
		for (qpl::size i = 0u; i < this->bet_types.size(); ++i) {
			if (this->bet_types[i].slider.value_was_modified()) {
				this->selected_money = this->bet_types[i].slider.get_value();
				this->slider_changed = true;
				this->selected_snail = i;
				this->bet_types[i].slider_text.set_string(qpl::to_string("Einsatz: ", this->bet_types[i].slider.get_value()));
				this->bet_types[i].leaf.set_center(this->bet_types[i].slider_text.get_visible_hitbox().middle_right().moved(this->leaf_shift, 0));
				if (this->selected_money) {
					this->bet_types[i].snail_border.set_color(this->bet_types[i].color.intensified(0.5).lighted(0.5));
				}
				else {
					this->bet_types[i].snail_border.set_color(this->bet_types[i].color.darkened(0.8));
				}
				index = i;
			}
		}
		if (index != qpl::size_max) {
			for (qpl::size i = 0u; i < this->bet_types.size(); ++i) {
				if (i != index) {
					this->bet_types[i].slider.set_value(0u);
					this->bet_types[i].slider_text.set_string(qpl::to_string("Einsatz: 0"));
					this->bet_types[i].leaf.set_center(this->bet_types[i].slider_text.get_visible_hitbox().middle_right().moved(this->leaf_shift, 0));
					this->bet_types[i].snail_border.set_color(this->bet_types[i].color.darkened(0.8));
				}
			}
		}
	}


	void draw(qsf::draw_object& draw) const {
		draw.draw(this->bet_types);
		draw.draw(this->seperations);
	}

	std::vector<bet_type> bet_types;
	std::vector<qsf::rectangle> seperations;
	qpl::animation animation;
	constexpr static auto leaf_shift = 30;
	qpl::size selected_money = 0u;
	qpl::size selected_snail = 0u;
	bool slider_changed = false;
	bool race_visible = false;
	bool race_finished = false;
	double goal = 0.0;
};
