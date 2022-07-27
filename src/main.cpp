
#include <qpl/qpl.hpp>

//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

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
	}

	void set_dimension(qpl::vector2f dimension) {
		this->background.set_dimension(qpl::vec(dimension.x, 120));
		this->background.set_color(qpl::rgb(20, 20, 20));
		this->background.set_outline_thickness(5.0f);
		this->background.set_outline_color(qpl::rgb(150, 150, 150));
	}

	void set_info(qpl::size amount) {
		this->text.set_string("Du hast ", amount);
		auto hitbox = this->text.get_visible_hitbox();
		auto pos = hitbox.middle_right().moved(40, 0);
		this->sprite.set_center(pos);
	}

	qsf::rectangle background;
	qsf::text text;
	qsf::sprite sprite;
};

struct menu_state : qsf::base_state {
	void init() override {
		this->buttons.resize(2u);
		for (qpl::size i = 0u; i < this->buttons.size(); ++i) {
			this->buttons[i].set_text_font("gidugu");
			this->buttons[i].set_text_character_size(60);
			this->buttons[i].set_background_color(qpl::rgb(20, 20, 40, 150));
			this->buttons[i].set_background_outline_thickness(2.0);
			this->buttons[i].set_background_outline_color(qpl::rgb::white);
		}
		this->background.set_texture(this->get_texture("menu"));
		this->background.set_scale(1.5);
		this->call_on_resize();
	}
	void call_on_activate() override {
		this->transition.make_appear();
		this->lock = false;
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
				}
				else if (button.get_text_string() == this->names[1]) {
					this->close();
				}
			}
		}
		if (this->transition.just_finished_disappearing()) {
			this->pop_this_state();
		}

		
	}
	void drawing() override {
		this->draw(this->background);
		this->draw(this->buttons);
		this->draw(this->transition);
	}

	constexpr static auto names = std::array{
		"SPIELEN", "BEENDEN"
	};
	std::vector<qsf::smooth_button> buttons;
	qsf::sprite background;
	qsf::transition_overlay transition;
	bool lock = false;
};

struct race_info {

	void draw(qsf::draw_object& draw) const {
		draw.draw(this->text_rectangle);
		draw.draw(this->text);
	}

	void init() {
		this->text.set_font("gidugu");
		this->text.set_character_size(150u);
		this->text.set_color(qpl::rgb(255, 150, 200));

		this->animation.set_duration(1.0);

		this->text_rectangle.set_color(qpl::rgb::transparent);
		this->text_rectangle.set_outline_thickness(4.0f);
		this->text_rectangle.set_slope_dimension(40);
		this->text_rectangle.set_outline_color(qpl::rgb(255, 200, 230));
	}
	void update(const qsf::event_info& event) {
		this->animation.update();
		if (this->animation.is_running()) {
			if (this->animation.just_finished_no_reverse()) {
				qpl::start_timed_task(0.3, [&]() {
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
	void set_dimension(qpl::vector2f dimension) {
		this->dimension = dimension;
		this->set_info(this->text.get_string());
		this->text_rectangle.set_center(this->dimension / 2);
	}
	void set_info(std::string text) {
		this->text.set_string(text);
		this->text.set_center(this->dimension / 2);
		this->text_rectangle.set_hitbox(this->text.get_visible_hitbox().increased(50));
	}

	qsf::text text;
	qsf::smooth_rectangle text_rectangle;
	qpl::vector2f dimension;
	qpl::animation animation;
};

struct player {
	qpl::size money = 50u;
	qpl::size money_before = 50u;
};

struct slug_info {
	std::string name;
	std::string rasse;
	double velocity1;
	double velocity2;
	qpl::rgb color;
	double progress = 0.0;
	double start = 0.0;
	bool at_goal = false;
	qpl::cubic_generator velocity_generator;

	slug_info(std::string name, std::string rasse, double vel1, double vel2, qpl::rgb color) {
		this->name = name;
		this->rasse = rasse;
		this->velocity1 = vel1;
		this->velocity2 = vel2;
		this->color = color;

		this->velocity_generator.set_random_range(this->velocity1, this->velocity2);
		this->velocity_generator.set_speed(0.8);
	}

	std::string string() const {
		return qpl::to_string(
			"Name    : ", this->name,
			"\nRasse   : ", this->rasse,
			"\nGeschw. : ", this->velocity1, " - ", this->velocity2, " m/h");
	}
	void update(const qsf::event_info& event, double goal) {
		auto f = event.frame_time_f();
		this->velocity_generator.update(f);

		auto vel = this->velocity_generator.get();

		auto speed = (vel / 3.6) * f;
		auto pixels = speed * 100;
		this->progress += pixels;
		if (this->progress > goal * 10'000) {
			this->progress = goal * 10'000;
			this->at_goal = true;
		}
	}
	void reset() {
		this->progress = 0.0;
		this->start = 0.0;
		this->at_goal = false;
	}
	qpl::f64 get_position() const {
		return this->start + this->progress;
	}
};

struct slug_interface {
	struct bet_type {
		qsf::sprite slug;
		qsf::sprite slug_border;
		qsf::sprite leaf;
		qsf::slider<qpl::u32> slider;
		qsf::text slider_text;
		qsf::text description_text;
		qsf::smooth_rectangle description_rectangle;
		qpl::rgb color;

		void draw(qsf::draw_object& draw) const {
			draw.draw(this->slug_border);
			draw.draw(this->slug);
			draw.draw(this->slider);
			draw.draw(this->slider_text);
			draw.draw(this->description_rectangle);
			draw.draw(this->description_text);
			draw.draw(this->leaf);
		}

		void update(const qsf::event_info& event) {
			event.update(this->slider);
		}
	};

	void create(const std::vector<slug_info>& slugs, const player& player) {
		this->bet_types.resize(slugs.size());
		for (qpl::size i = 0u; i < slugs.size(); ++i) {
			auto scale = 0.3;

			this->bet_types[i].color = slugs[i].color;
			this->bet_types[i].slug.set_texture(qsf::get_texture("slug"));
			this->bet_types[i].slug.set_scale(scale);

			this->bet_types[i].leaf.set_texture(qsf::get_texture("leaf"));
			this->bet_types[i].leaf.set_scale(0.1);

			this->bet_types[i].slug_border.set_texture(qsf::get_texture("slug_border"));
			this->bet_types[i].slug_border.set_scale(scale);

			this->bet_types[i].slug.set_color(slugs[i].color);

			this->bet_types[i].slider_text.set_font("gidugu");
			this->bet_types[i].slider_text.set_character_size(50);
			this->bet_types[i].slider_text.set_string("Einsatz: 0");


			this->bet_types[i].description_text.set_font("anonymous");
			this->bet_types[i].description_text.set_character_size(23);
			this->bet_types[i].description_text.set_string(slugs[i].string());
			this->bet_types[i].description_text.set_line_spacing(0.9f);
			this->bet_types[i].description_text.set_color(qpl::rgb::white);

			this->bet_types[i].slider.set_dimensions({ 300, 40 }, { 30, 40 });
			this->bet_types[i].slider.set_range(0u, player.money, 0u);
			this->bet_types[i].slider.set_hover_increase(1);

			this->bet_types[i].description_rectangle.set_color(qpl::rgb(20, 20, 20));
			this->bet_types[i].description_rectangle.set_outline_thickness(3.0f);
			this->bet_types[i].description_rectangle.set_outline_color(slugs[i].color.with_alpha(50));
		}

		this->seperations.resize(slugs.size() - 1);
		for (auto& i : this->seperations) {
			i.set_color(qpl::rgb(200, 200, 200));
		}

		this->reset_visibility();
	}

	void reset_visibility() {
		for (auto& i : this->bet_types) {
			i.slider.set_alpha(0);
			i.slug.set_multiplied_alpha(0);
			i.slug_border.set_multiplied_alpha(0);
			i.slider_text.set_multiplied_alpha(0);
			i.description_rectangle.set_multiplied_alpha(0);
			i.description_text.set_multiplied_alpha(0);
			i.leaf.set_multiplied_alpha(0);
		}
		for (auto& i : this->seperations) {
			i.set_multiplied_alpha(0);
		}
		this->race_visible = false;
	}

	void set_positions(qpl::vector2f dimension) {

		auto diff = 150 * ((this->bet_types.size() - 1) / 2.0f);
		auto start = qpl::vec(350, (dimension.y / 2) - diff);
		for (qpl::size i = 0u; i < this->bet_types.size(); ++i) {
			auto pos = start + qpl::vec(0, 150 * i);

			this->bet_types[i].slug.set_position(pos);
			this->bet_types[i].slug.centerize_y();
			this->bet_types[i].slug_border.set_position(pos);
			this->bet_types[i].slug_border.centerize_y();
			this->bet_types[i].slider.set_center(pos + qpl::vec(350, 0));

			this->bet_types[i].slider_text.set_center(pos + qpl::vec(200 + 400, 0));

			this->bet_types[i].slider_text.set_position(pos + qpl::vec(200 + 400, 0));
			this->bet_types[i].slider_text.centerize_y();

			this->bet_types[i].leaf.set_center(this->bet_types[i].slider_text.get_visible_hitbox().middle_right().moved(this->leaf_shift, 0));

			this->bet_types[i].description_text.set_center(pos - qpl::vec(170, 0));

			auto h = this->bet_types[i].description_text.get_visible_hitbox();
			this->bet_types[i].description_rectangle.set_hitbox(h.increased(10));

			if (i < this->seperations.size()) {
				this->seperations[i].set_position(qpl::vec(50, pos.y + 70));
				this->seperations[i].set_dimension(qpl::vec(dimension.x - 100, 1));
			}
		}
	}

	void apply_progress(const std::vector<slug_info>& slugs) {
		for (qpl::size i = 0u; i < slugs.size(); ++i) {
			this->bet_types[i].slug.set_position_x(slugs[i].get_position());
			this->bet_types[i].slug_border.set_position_x(slugs[i].get_position());
		}
	}
	
	void fade_in() {
		this->animation.reset_and_start();
	}
	void fade_out() {
		this->animation.reset_and_start_reverse();
	}


	void update(const qsf::event_info& event) {

		this->slider_changed = false;

		this->animation.update();
		if (this->animation.is_running()) {
			auto p = this->animation.get_curve_progress();
			auto a = qpl::u8_cast(255 * p);
			for (auto& i : this->bet_types) {

				if (!this->race_visible) {
					i.slug.set_multiplied_alpha(a);
					i.slug_border.set_multiplied_alpha(a);
				}
				i.description_text.set_multiplied_alpha(a);
				i.description_rectangle.set_multiplied_alpha(a);
				i.slider.set_alpha(a);
				i.slider_text.set_multiplied_alpha(a);
				i.leaf.set_multiplied_alpha(a);
			}
			for (auto& i : this->seperations) {
				i.set_multiplied_alpha(a);
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
				this->bet_types[i].slider_text.set_string(qpl::to_string("Einsatz: ", this->bet_types[i].slider.get_value()));
				this->bet_types[i].leaf.set_center(this->bet_types[i].slider_text.get_visible_hitbox().middle_right().moved(this->leaf_shift, 0));
				if (this->selected_money) {
					this->bet_types[i].slug_border.set_color(this->bet_types[i].color.intensified(0.5));
				}
				else {
					this->bet_types[i].slug_border.set_color(qpl::rgb::white);
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
					this->bet_types[i].slug_border.set_color(qpl::rgb::white);
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
	bool slider_changed = false;
	bool race_visible = false;
};

struct race_distance {
	
	void create(qpl::size slug_count, double goal) {
		this->slug_count = slug_count;
		this->goal = goal;
	}
	void init() {
		this->line.set_color(qpl::rgb(200, 200, 200));
		this->start_line.set_color(qpl::rgb(50, 150, 50));
		this->finish_line.set_color(qpl::rgb(150, 50, 50));

		this->tile_map.set_texture(qsf::get_texture("grass"), 32);
	}
	void set_invisible() {
		this->line.set_multiplied_alpha(0);
		this->start_line.set_multiplied_alpha(0);
		this->finish_line.set_multiplied_alpha(0);
		for (auto& i : this->seperations) {
			i.set_multiplied_alpha(0);
		}
		for (auto& i : this->tile_map.vertices) {
			i.color = qpl::rgb::white.with_alpha(0);
		}
	}
	void set_dimension(qpl::vector2f dimension) {

		constexpr auto slug_offset = 0;
		constexpr auto slug_width = 500 * 0.3;

		auto diff = 150 * ((this->slug_count - 1) / 2.0f);
		auto pos = qpl::vec(350 + slug_offset, (dimension.y / 2) - diff);
		pos.y += this->slug_count * 150 - 50;

		auto dim = qpl::vec(this->goal * 10'000 + slug_width, 5);
		this->line.set_position(pos);
		this->line.set_dimension(dim);

		auto line_height = this->slug_count * 150;
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
	void fade_out() {
		this->animation.reset_and_start_reverse();
	}

	void update(const qsf::event_info& event) {
		this->animation.update();
		if (this->animation.is_running()) {
			auto p = this->animation.get_curve_progress();
			auto a = qpl::u8_cast(p * 255);

			this->line.set_multiplied_alpha(a);
			this->start_line.set_multiplied_alpha(a);
			this->finish_line.set_multiplied_alpha(a);
			for (auto& i : this->seperations) {
				i.set_multiplied_alpha(a);
			}
			for (auto& i : this->tile_map.vertices) {
				i.color = qpl::rgb::white.with_alpha(a);
			}
		}
	}

	void draw(qsf::draw_object& draw) const {
		draw.draw(this->tile_map);
		draw.draw(this->line);
		draw.draw(this->start_line);
		draw.draw(this->finish_line);
	}

	std::vector<qsf::rectangle> seperations;
	qsf::rectangle line;
	qsf::rectangle start_line;
	qsf::rectangle finish_line;
	qpl::size slug_count = 0u;
	qsf::small_tile_map tile_map;
	qpl::animation animation;
	double goal = 0.0;
};

struct game_state : qsf::base_state {
	void init() override {
		this->hud.init();
		this->hud.set_info(50);

		this->race_info.init();
		this->race_info.set_info("RENNEN #1");

		this->background.set_texture(this->get_texture("game"));

		this->slugs = {
			{"Silly", "Gelbschnecke", 2.5, 3.5, qpl::rgb(255, 200, 100) },
			{"Rolly", "Blauschnecke", 2.9, 3.1, qpl::rgb(100, 200, 255) },
			{"Killy", "Rotschnecke", 2.0, 4.0, qpl::rgb(255, 100, 100) },
		};

		this->slug_interface.create(this->slugs, this->player);

		this->race_distance.init();
		this->race_distance.create(this->slugs.size(), 40.0 / 1000);

		this->call_on_resize();

		this->start_button.set_text_font("gidugu");
		this->start_button.set_text_character_size(50);
		this->start_button.set_text_string("SETZE WETTE");
		this->start_button.set_background_color(qpl::rgb(20, 20, 40, 150));
		this->start_button.set_background_outline_thickness(2.0);
		this->start_button.set_background_outline_color(qpl::rgb::white);
		this->start_button.set_dimension({ 250, 70 });

	}
	void call_on_resize() override {
		this->hud.set_dimension(this->dimension());
		this->race_info.set_dimension(this->dimension());
		this->background.set_center(this->center());
		this->transition.set_dimension(this->dimension());
		this->slug_interface.set_positions(this->dimension());

		for (qpl::size i = 0; i < this->slug_interface.bet_types.size(); ++i) {
			this->slugs[i].start = this->slug_interface.bet_types[i].slug.get_position().x;
		}

		auto dim = this->start_button.get_dimension();
		this->start_button.set_position(this->dimension() - dim - qpl::vec(30, 30));
		this->start_button.centerize_text();
		this->race_distance.set_dimension(this->dimension());
	}
	void call_on_activate() override {
		this->race_info.start_animation();
		this->transition.make_appear();
		this->slug_interface.reset_visibility();
		this->start_button_animation.reset();
		this->start_button_visible = false;
		this->start_button.set_multiplied_alpha(0);
		for (auto& i : this->slugs) {
			i.reset();
		}
		this->slug_interface.set_positions(this->dimension());
		this->race_distance.set_invisible();
		this->racing = false;
	}
	void updating() override {
		this->update(this->race_info);
		this->update(this->transition);
		this->update(this->slug_interface);
		this->update(this->start_button);
		this->update(this->race_distance);

		if (this->start_button_visible && this->start_button.is_clicked()) {
			this->slug_interface.race_visible = true;
			this->slug_interface.fade_out();
			this->start_button_animation.reset_and_start_reverse();
		}
		if (this->start_button_animation.just_finished_reverse()) {
			this->racing = true;
		}

		this->start_button_animation.update();
		if (this->start_button_animation.is_running()) {
			auto p = this->start_button_animation.get_curve_progress();
			auto a = qpl::u8_cast(255 * p);
			this->start_button.set_multiplied_alpha(a);
		}
		if (this->start_button_animation.just_finished_reverse()) {
			this->start_button_visible = false;
			this->race_distance.fade_in();
		}

		if (this->slug_interface.slider_changed) {
			this->player.money = this->player.money_before - this->slug_interface.selected_money;
			this->hud.set_info(this->player.money);

			if (this->slug_interface.selected_money) {
				this->start_button_animation.go_forwards();
				this->start_button_visible = true;
			}
			else if (!this->slug_interface.selected_money && this->start_button_visible) {
				this->start_button_animation.go_backwards();
			}
		}

		if (this->race_info.animation.just_finished_reverse()) {
			this->slug_interface.fade_in();
		}

		if (this->event().key_pressed(sf::Keyboard::Escape)) {
			this->transition.make_disappear();
		}

		if (this->transition.just_finished_disappearing()) {
			this->add_state<menu_state>();
		}

		if (this->racing) {
			for (auto& i : this->slugs) {
				i.update(this->event(), this->race_distance.goal);
			}
			this->slug_interface.apply_progress(this->slugs);
		}
	}
	void drawing() override {
		this->draw(this->background);
		this->draw(this->race_distance);
		this->draw(this->hud);
		this->draw(this->race_info);
		this->draw(this->slug_interface);
		this->draw(this->start_button);

		this->draw(this->transition);
	}

	hud hud;
	race_info race_info;
	qsf::sprite background;
	qsf::transition_overlay transition;
	slug_interface slug_interface;
	player player;
	qsf::smooth_button start_button;
	qpl::animation start_button_animation;
	std::vector<slug_info> slugs; 
	race_distance race_distance;
	bool start_button_visible = false;
	bool racing = false;
};

int main() try {

	qsf::framework framework;
	framework.set_title("QPL");
	framework.set_framerate_limit(60);
	framework.add_font("gidugu", "resources/Gidugu-Regular.ttf");
	framework.add_font("anonymous", "resources/AnonymousPro-Bold.ttf");
	framework.add_texture("slug", "resources/slug.png");
	framework.add_texture("slug_border", "resources/slug_border.png");
	framework.add_texture("leaf", "resources/leaf.png");
	framework.add_texture("menu", "resources/menu.png");
	framework.add_texture("game", "resources/game.png");
	framework.add_texture("grass", "resources/grass.png");
	framework.set_dimension({ 1400u, 950u });

	framework.add_state<game_state>();
	framework.add_state<menu_state>();
	framework.game_loop();
}
catch (std::exception& any) {
	qsf::run_exception_window(any.what(), "resources/AnonymousPro-Regular.ttf");
}