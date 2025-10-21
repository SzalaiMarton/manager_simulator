#pragma once

#include "logging.h"
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <random>
#include <set>
#include <numeric>

namespace Traits {
	enum CharacterPersonalities {
		LEADER,
		CAREGIVER,
		THINKER,
		ADVENTURER,
		ORGANIZER,
		PEACEMAKER,
		DREAMER,
		PERFORMER,
		LOYALIST,
		PERSONALITY_NONE
	};

	enum CharacterEmotions {
		ANGRY,
		FINE,
		RELAXED,
		SAD,
		CONFUSED,
		INSPIRED,
		EMOTION_NONE
	};

	enum CharacterMotivations {
		POWER,
		WEALTH,
		LOVE,
		KNOWLEDGE,
		FREEDOM,
		RESTRICTED,
		SAFETY,
		FAIRNESS,
		MOTIVATION_NONE
	};

	enum CharacterMorality {
		LOYAL,
		TRUSTING,
		COOPERATIVE,
		PROTECTIVE,
		CHARMING,
		SELFLESS,
		SELFISH,
		SUSPICIOUS,
		COMPETETIVE,
		NEGLECTFUL,
		AWKWARD,
		MORALITY_NONE
	};

	enum CharacterIntelligence {
		SMART,
		CREATIVE,
		SKILLED,
		STRATEGIC,
		NAIVE,
		PRACTICAL,
		CLUMSY,
		RECKLESS,
		INTELLIGENCE_NONE
	};

	enum CharacterBackground {
		WEALTHY,
		EDUCATED,
		RURAL,
		INDEPENDENT,
		RELIGIOUS,
		POOR,
		UNEDUCATED,
		URBAN,
		DEPENDENT,
		SECULAR,
		BACKGROUND_NONE
	};

	CharacterBackground stringToBackground(const std::string& background);
	CharacterEmotions stringToEmotion(const std::string& emotion);
	CharacterIntelligence stringToIntelligence(const std::string& intelligence);
	CharacterMorality stringToMorality(const std::string& morality);
	CharacterMotivations stringToMotivation(const std::string& motivation);
	CharacterPersonalities stringToPersonalities(const std::string& personality);
	std::string traitToString(CharacterPersonalities personality = PERSONALITY_NONE, CharacterMotivations motivation = MOTIVATION_NONE, 
		CharacterMorality morality = MORALITY_NONE, CharacterIntelligence intelligenece = INTELLIGENCE_NONE, CharacterEmotions emotion = EMOTION_NONE,
		CharacterBackground background = BACKGROUND_NONE);
}

namespace CharacterConfig {
	struct Config {
		std::unordered_map<std::string, std::vector<std::string>> categories;
		std::unordered_map<std::string, std::unordered_map<std::string, double>> base;
		struct PersonaMods {
			std::unordered_map<std::string, std::unordered_map<std::string, double>> mult;
			void print();
		};
		std::unordered_map<std::string, PersonaMods> personas;

		double persona_mult_weight = 1.0;
		double add_noise_sigma = 0.05;
		double floor = 0.001;

		void print();
	};
	Config loadConfig(const std::string& file);

	extern Config config;
	constexpr uint8_t numberOfTraits = 3;
}

class TraitGenerator {
public:
	explicit TraitGenerator(unsigned int seed = 123456789) : rng(seed), dist(0.0, 1.0) {
	}
	std::string pickTraitForCategory(const CharacterConfig::Config& cfg, const std::string& persona, const std::string& category);
	std::unordered_map<std::string, std::set<std::string>> generateCharacterSheet(const CharacterConfig::Config& cfg, const std::string& persona, unsigned int seed);

	
private:
	std::mt19937 rng;
	std::uniform_real_distribution<double> dist;

	int uniformIndex(size_t n);
	double normalNoise(double sigma);
};


class Character {
public:
	std::string* name;

	// for the user these are hidden
	Traits::CharacterPersonalities personality;

	// these are based on the character's personality
	std::vector<Traits::CharacterEmotions> currentEmotions;
	std::vector<Traits::CharacterMotivations> motivations;
	std::vector<Traits::CharacterMorality> moralities;
	std::vector<Traits::CharacterIntelligence> intelligence;
	std::vector<Traits::CharacterBackground> background;

	Character() = default;
	Character(const std::string& persona, const CharacterConfig::Config& cfg, TraitGenerator* traitGen);
	~Character();
};

namespace Names {
	static bool names_1 = false;
	static bool names_2 = false;
	static bool names_3 = false;
	static bool names_4 = false;
	static bool names_5 = false;

	static std::vector<std::string*> names = {};

	std::string* getRandomName();
	void loadRandomNames();
	void deloadNames();
}