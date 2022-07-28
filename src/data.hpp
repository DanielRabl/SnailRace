#pragma once

namespace data {
	constexpr auto snail_offset = 0;
	constexpr auto snail_width = 500 * 0.3;
	constexpr auto hud_height = 120;
	constexpr auto snail_delta_y = 130;
	constexpr auto snail_start_x = 380;

	constexpr auto get_center_after_hud(qpl::vector2f dimension) {
		return (dimension - qpl::vec(0, data::hud_height)) / 2 + qpl::vec(0, data::hud_height);
	}
}