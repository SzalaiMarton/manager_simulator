#pragma once

#include "settings.h"

namespace Game {
	class Game {
	public:
		CharacterConfig::Config characterConfig;
		TraitGenerator traitGenerator;

		std::vector<SaveCreator::Save*> saves;

		Graphics::Background* mainMenuBG;
		Graphics::Background* onScreenBG;

		Graphics::Renderer* renderer;


		Game();
		~Game() = default;

		void initMainMenu();
		void initSavesMenu();
		void initNewGameMenu();

		void run();
		void shutdown();
	};
}