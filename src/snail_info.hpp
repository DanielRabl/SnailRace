#pragma once

#include <qpl/qpl.hpp>

struct snail_info {
	std::string name;
	std::string rasse;
	double velocity1;
	double velocity2;
	double velocity;
	qpl::rgb color;
	double progress = 0.0;
	double start = 0.0;
	bool at_goal = false;
	bool just_reached_goal = false;
	qpl::cubic_generator velocity_generator;

	snail_info(std::string name, std::string rasse, double vel1, double vel2, qpl::rgb color) {
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

		this->velocity = this->velocity_generator.get();

		auto before = this->at_goal;
		auto speed = (this->velocity / 3.6) * f;
		auto pixels = speed * 100;
		this->progress += pixels;
		if (this->progress > goal * 10'000) {
			this->progress = goal * 10'000;
			this->at_goal = true;
		}

		this->just_reached_goal = !before && this->at_goal;
	}
	void reset() {
		this->progress = 0.0;
		this->start = 0.0;
		this->at_goal = false;
		this->velocity_generator.update(qpl::random(0.0, 1.0));
	}
	qpl::f64 get_position() const {
		return this->start + this->progress;
	}
};