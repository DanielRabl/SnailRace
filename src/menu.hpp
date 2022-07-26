#pragma once

#include <qpl/qpl.hpp>
#include "game.hpp"
#include "credits.hpp"

struct menu_state : qsf::base_state {
	void init() override {
		this->buttons.resize(3u);
		for (qpl::size i = 0u; i < this->buttons.size(); ++i) {
			this->buttons[i].set_text_font("gidugu");
			this->buttons[i].set_text_character_size(60);
			this->buttons[i].set_background_color(qpl::rgba(20, 20, 40, 150));
			this->buttons[i].set_background_outline_thickness(2.0);
			this->buttons[i].set_background_outline_color(qpl::rgb::white);
		}
		this->background.set_texture(this->get_texture("menu"));
		this->background.set_scale(1.5);
		this->call_on_resize();

		this->framework->set_icon("resources/logo.png");

	}
	void call_on_activate() override {
		this->transition.make_appear();
		this->lock = false;
		this->push_game = false;
	}
	void call_on_resize() override {

		auto diff = 350 * ((this->buttons.size() - 1) / 2.0f);
		auto start = this->center() - qpl::vec(diff, 200);
		for (qpl::size i = 0u; i < this->buttons.size(); ++i) {
			this->buttons[i].set_dimension({ 300, 60 });
			this->buttons[i].set_center(start + qpl::vec(350 * i, 0));
			this->buttons[i].set_text_string(this->names[i]);
			this->buttons[i].centerize_text();
		}
		this->background.set_center(this->center());
		this->transition.set_dimension(this->dimension());
	}
	void updating() override {
		this->update(this->buttons);
		this->update(this->transition);

		for (auto& button : this->buttons) {
			if (!this->lock && button.is_clicked()) {
				if (button.get_text_string() == this->names[0]) {
					this->transition.make_disappear();
					this->lock = true;
					this->push_game = true;
				}
				else if (button.get_text_string() == this->names[1]) {
					this->transition.make_disappear();
					this->lock = true;
					this->push_game = false;
				}
				else if (button.get_text_string() == this->names[2]) {
					this->close();
				}
			}
		}
		if (this->transition.just_finished_disappearing()) {
			if (this->push_game) {
				this->add_state<game_state>();
			}
			else {
				this->add_state<credit_state>();
			}
		}


	}
	void drawing() override {
		this->draw(this->background);
		this->draw(this->buttons);
		this->draw(this->transition);
	}

	constexpr static auto names = std::array{
		"PLAY", "CREDITS", "EXIT"
	};
	std::vector<qsf::smooth_button> buttons;
	qsf::sprite background;
	qsf::transition_overlay transition;
	bool lock = false;
	bool push_game = false;
};