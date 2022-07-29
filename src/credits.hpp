#pragma once

#include <qpl/qpl.hpp>

struct credit_state : qsf::base_state {
	void init() override {
		this->text.set_font("gidugu");
		this->text.set_character_size(50);

		this->clear_color = qpl::rgb(10, 10, 15);
		this->border.set_texture(qsf::get_texture("shadow"));
		this->border.set_color(this->clear_color);
		this->border.set_scale(0.5);


		std::string string;
		this->text.set_line_spacing(0.7);
		this->text.set_position({ 100, 100 });
		this->text << "QPL GitHub: " << qsf::ts::push() << qsf::ts::color(qpl::rgb(100, 100, 255)) << "https://github.com/DanielRabl/QPL\n" << qsf::ts::pop();
		this->text << "Framework: QPL by Daniel Rabl\n";
		this->text << "Library: SFML\n";
		this->text << "API: OpenGL\n";
		this->text << "GitHub: " << qsf::ts::push() << qsf::ts::color(qpl::rgb(100, 100, 255)) << "https://github.com/DanielRabl/SnailRace\n" << qsf::ts::pop();
		this->text << "Created by: Daniel Rabl\n";
		this->text << "Created: 29.07.2022, Würzburg\n";

		this->call_on_resize();
	}



	void call_on_activate() override {

		this->text.set_position(qpl::vec(200, -380));

		this->transition.make_appear();
		this->lock = false;
	}
	void call_on_resize() override {
		this->transition.set_dimension(this->dimension());
		this->border.set_dimension(this->dimension());
		this->border.clear();
		this->border.add_top(-0.5);
		this->border.add_bottom(-0.5);
	}
	void updating() override {
		this->update(this->transition);

		this->text.move(qpl::vec(0, this->event().frame_time_f() * 20));

		if (this->event().key_pressed(sf::Keyboard::Escape) && !this->lock) {
			this->transition.make_disappear();
			this->lock = true;
		}

		if (this->transition.just_finished_disappearing()) {
			this->pop_this_state();
		}


	}
	void drawing() override {
		this->draw(this->text);
		this->draw(this->transition);
		this->draw(this->border);
	}

	qsf::transition_overlay transition;
	qsf::border_graphic border;
	qsf::text_stream text;
	bool lock = false;
};