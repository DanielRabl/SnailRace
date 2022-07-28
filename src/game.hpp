#pragma once

#include <qpl/qpl.hpp>
#include "hud.hpp"
#include "race_floor.hpp"
#include "race.hpp"
#include "player.hpp"
#include "race_announcement.hpp"
#include "win_announcement.hpp"
#include "snail_interface.hpp"
#include "start_button.hpp"
#include "bet_feedback.hpp"

struct game_state : qsf::base_state {
	void init() override {
		this->hud.init();
		this->hud.set_info(50);

		this->race_announcement.init();

		this->win_announcement.init();
		this->win_announcement.set_info("TEST");

		this->background.set_texture(this->get_texture("game"));
		this->bet_feedback.init();

		this->races = { 
			::race{ {
				{"Silly", "Gelbschnecke", 2.5, 3.5,     qpl::rgb(255, 200, 100) },
				{"Sally", "Blauschnecke", 1.5, 4.5,     qpl::rgb(150, 150, 255) },
				{"Killy", "Rotschnecke",  2.0, 4.0,     qpl::rgb(255, 100, 100) },
			}, 40.0 / 1000 },
			::race{ {
				{"Willy", "Cyanschnecke",  2.3, 3.7,    qpl::rgb(100, 255, 255) },
				{"Olly", "Braunschnecke",  2.75, 3.25,  qpl::rgb(150, 70, 40)},
				{"Tolly", "Grünschnecke",  1.5, 4.5,    qpl::rgb(50, 255, 50) },
			}, 50.0 / 1000 },
			::race{ {
				{"Qilly", "Grasschnecke",    2.4, 3.6,  qpl::rgb(180, 255, 0) },
				{"Kolly", "Pinkschnecke",    2.8, 3.2,  qpl::rgb(255, 100, 255)},
				{"Gilly", "Goldschnecke",    1.5, 4.5,  qpl::rgb(255, 255, 50) },
				{"Nelly", "Lilaschnecke",    1.5, 4.5,  qpl::rgb(200, 100, 255) },
				{"Rolly", "Silberschnecke",  2.9, 3.1,  qpl::rgb(200, 255, 255) },
			}, 60.0 / 1000 },
		};

		this->race_floor.init();
		this->start_button.init();
		this->set_speed_factor(1.0);

		this->graph.set_font("gidugu");
		this->graph.enable_axis_info();
		this->graph.background_color = qpl::rgb(20, 20, 20, 150);
	}
	void call_on_resize() override {
		this->hud.set_dimension(this->dimension());
		this->race_announcement.set_dimension(this->dimension());
		this->win_announcement.set_dimension(this->dimension());
		this->background.set_center(this->center());
		this->transition.set_dimension(this->dimension());
		this->snail_interface.set_positions(this->dimension());
		this->race.set_positions(this->snail_interface);
		this->start_button.set_dimension(this->dimension());
		this->race_floor.set_dimension(this->dimension());
		this->race_floor.apply_animation();
		this->bet_feedback.set_dimension(this->dimension());

		this->graph.set_dimension(this->dimension() - 200);
		this->graph.set_position(qpl::vec(100, 100));
	}
	void reset() {
		this->racing = false;
		this->clicked_start = false;
		this->won = false;
		this->winners_count = 0u;
	}
	void call_on_activate() override {
		this->current_race = 0u;
		this->first_winner = qpl::size_max;

		this->race = this->races[this->current_race];
		this->snail_interface.create(this->race.snails, this->player, this->race.goal);
		this->snail_interface.set_positions(this->dimension());
		this->snail_interface.reset_visibility();
		this->snail_interface.reset(this->player);


		this->race_floor.create(this->race.snails.size(), this->race.goal);
		this->race_floor.reset();
		this->race_floor.apply_animation();

		this->win_announcement.animation.reset();
		this->win_announcement.apply_animation();
		this->transition.make_appear();
		this->start_button.reset();
		for (auto& i : this->race.snails) {
			i.reset();
		}
		this->bet_feedback.reset();
		this->reset();

		this->hud.reset();

		this->race_announcement.set_info("RENNEN #1");
		this->race_announcement.start_animation();
		this->race_announcement.set_colors();

		this->call_on_resize();

	}
	void updating() override {
		if (this->event().key_single_pressed(sf::Keyboard::G)) {
			this->graph_visible = !this->graph_visible;
		}

		if (this->graph_visible) {
			this->update(this->graph);
		}
		this->update(this->race_announcement);
		this->update(this->win_announcement);
		this->update(this->transition);
		this->update(this->snail_interface);
		this->update(this->start_button);
		this->update(this->race_floor);
		this->update(this->bet_feedback);
		this->update(this->hud);

		if (this->start_button.visible && this->start_button.button.is_clicked() && this->snail_interface.selected_money) {
			this->snail_interface.race_visible = true;
			this->clicked_start = true;
			this->snail_interface.fade_out();
			this->start_button.animation.reset_and_start_reverse();
		}
		this->start_button.update_animation(this->event());
		if (this->start_button.animation.just_finished_reverse() && this->clicked_start) {
			this->start_button.visible = false;
			this->race_floor.fade_in();
		}
		if (this->race_floor.finished_fading_in()) {
			this->racing = true;
			this->graph.clear_data();
		}

		if (this->snail_interface.slider_changed) {
			this->player.money = this->player.money_before - this->snail_interface.selected_money;

			if (this->snail_interface.selected_money) {
				this->start_button.animation.go_forwards();
				this->start_button.visible = true;
			}
			else if (!this->snail_interface.selected_money && this->start_button.visible) {
				this->start_button.animation.go_backwards();
			}
		}

		if (this->race_announcement.animation_finished()) {
			if (this->current_race < this->races.size()) {
				this->snail_interface.fade_in();
			}
			else {
				this->transition.make_disappear();
			}
		}

		if (this->event().key_pressed(sf::Keyboard::Escape)) {
			this->transition.make_disappear();
		}

		if (this->transition.just_finished_disappearing()) {
			this->add_state<menu_state>();
		}

		if (this->racing) {

			std::vector<qpl::f64> velocities;
			for (qpl::size i = 0u; i < this->race.snails.size(); ++i) {
				this->race.snails[i].update(this->event(), this->race_floor.goal);
				if (this->race.snails[i].just_reached_goal) {
					++this->winners_count;
					this->snail_interface.bet_types[i].fade_winner_in(this->winners_count);
					if (this->winners_count == 1u) {
						this->first_winner = i;
					}
					if (this->winners_count == this->race.snails.size()) {
						if (this->first_winner == this->snail_interface.selected_snail) {
							this->bet_feedback.set_text("Deine Wette hat gewonnen!");
							this->bet_feedback.set_color(qpl::rgb(100, 255, 100));
							this->won = true;
						}
						else {
							this->bet_feedback.set_text("Deine Wette hat verloren.");
							this->bet_feedback.set_color(qpl::rgb(255, 100, 100));
							this->won = false;
						}
						qpl::start_timed_task(1.5, [&]() {
							this->bet_feedback.fade_in();
							this->snail_interface.race_finished = true;
						});
						for (qpl::size i = 0u; i < this->race.snails.size(); ++i) {
							this->graph.get_standard_graph(this->race.snails[i].name).add_data(0);
						}
					}
				}

				qpl::f64 velocity = 0.0;
				if (!this->race.snails[i].at_goal) {
					if (this->graph.standard_graphs.find(this->race.snails[i].name) == this->graph.standard_graphs.cend()) {
						this->graph.get_standard_graph(this->race.snails[i].name).color = this->race.snails[i].color;
						this->graph.get_standard_graph(this->race.snails[i].name).thickness = 2.0;
					}
					velocity = this->race.snails[i].velocity;
				}
				velocities.push_back(velocity);
			}
			this->snail_interface.apply_progress(this->race.snails);


			if (this->winners_count != this->race.snails.size()) {
				for (qpl::size i = 0u; i < velocities.size(); ++i) {
					this->graph.get_standard_graph(this->race.snails[i].name).add_data(velocities[i]);
				}
			}
		}

		if (this->bet_feedback.animation.just_finished_no_reverse()) {
			if (this->won) {
				auto money = this->snail_interface.selected_money;
				auto win = money * this->snail_interface.bet_types.size();;
				this->player.money += win;
				this->player.money_before = this->player.money;

				this->win_announcement.set_info(qpl::to_string("Gewonnen: +", win));
				this->win_announcement.set_color(qpl::rgb(100, 255, 100));
			}
			else {
				this->win_announcement.set_info(qpl::to_string("Verloren: -", this->snail_interface.selected_money));
				this->win_announcement.set_color(qpl::rgb(255, 100, 100));
			}
		}
		if (this->bet_feedback.animation.just_finished_reverse()) {
			this->snail_interface.fade_out();
			this->race_floor.fade_out();
		}
		if (this->racing && this->snail_interface.finished_fading_out()) {
			++this->current_race;

			if (this->current_race < this->races.size()) {
				this->race = this->races[this->current_race];

				for (auto& i : this->race.snails) {
					i.reset();
				}


				this->snail_interface.create(this->race.snails, this->player, this->race.goal);
				this->snail_interface.set_positions(this->dimension());
				this->snail_interface.reset_visibility();
				this->snail_interface.reset(this->player);
				

				this->race.set_positions(this->snail_interface);

				this->reset();

				this->race_floor.create(this->race.snails.size(), this->race.goal);
				this->race_floor.set_dimension(this->dimension());
				this->race_floor.apply_animation();

			}
			this->win_announcement.start_animation();
		}

		if (this->win_announcement.animation.just_finished_no_reverse()) {
			this->hud.set_info(this->player.money);
			this->hud.start_animation(this->win_announcement.text.get_color());
		}
		if (this->win_announcement.animation_finished()) {
			if (this->current_race < this->races.size()) {
				this->race_announcement.set_info(qpl::to_string("RENNEN #", this->current_race + 1));
				this->race_announcement.start_animation();
			}
			else {
				this->race_announcement.set_info(qpl::to_string("SPIEL-SCORE = ", this->player.money));
				this->race_announcement.text_rectangle.set_outline_color(qpl::rgb::blue);
				this->race_announcement.start_animation();
			}
		}
	}
	void drawing() override {
		this->draw(this->background);
		this->draw(this->race_floor);
		this->draw(this->race_announcement);
		this->draw(this->win_announcement);
		this->draw(this->snail_interface);
		this->draw(this->start_button);
		this->draw(this->bet_feedback);
		this->draw(this->hud);

		if (this->graph_visible) {
			this->draw(this->graph);
		}

		this->draw(this->transition);
	}

	hud hud;
	race_announcement race_announcement;
	win_announcement win_announcement;
	qsf::sprite background;
	qsf::transition_overlay transition;
	snail_interface snail_interface;
	start_button start_button;
	bet_feedback bet_feedback;
	player player;
	std::vector<race> races;
	race race;
	qpl::size current_race = 0u;
	qsf::vgraph graph;



	race_floor race_floor;
	qpl::size winners_count = 0u;
	qpl::size first_winner = qpl::size_max;
	bool clicked_start = false;
	bool racing = false;
	bool won = false;
	bool graph_visible = false;
};
