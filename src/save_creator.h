#pragma once

namespace Modes {
	enum GameMode {
		E_SPORT,
		SPORT
	};

	enum EsportModes {
		MOBA,
		SHOOTER,
		NONE_ESPORT
	};

	enum SportModes {
		FOOTBALL,
		BASKETBALL,
		HANDBALL,
		NONE_SPORT
	};
}

namespace SaveCreator {
	class Save {
		Save() = default;
		Save(Modes::GameMode gameMode, Modes::EsportModes esportMode, Modes::SportModes sportMode);
		~Save() = default;
	};
}