
#include <qpl/qpl.hpp>
#include "menu.hpp"
#include "game.hpp"

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main() try {

	qsf::framework framework;
	framework.set_speed_factor(2);
	framework.set_title("Snail Race");
	framework.set_framerate_limit(60);
	framework.add_font("gidugu", "resources/Gidugu-Regular.ttf");
	framework.add_font("anonymous", "resources/AnonymousPro-Bold.ttf");
	framework.add_texture("snail", "resources/snail.png");
	framework.add_texture("snail_border", "resources/snail_border.png");
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