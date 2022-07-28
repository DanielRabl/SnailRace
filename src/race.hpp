#pragma once

#include <qpl/qpl.hpp>
#include "snail_info.hpp"
#include "snail_interface.hpp"

struct race {
	std::vector<snail_info> snails;
	qpl::f64 goal;

	void set_positions(const snail_interface& snail_interface) {
		for (qpl::size i = 0; i < snail_interface.bet_types.size(); ++i) {
			this->snails[i].start = snail_interface.bet_types[i].snail.get_position().x;
		}
	}
};