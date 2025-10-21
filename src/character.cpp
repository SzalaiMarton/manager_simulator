#include "character.h"
#include "logging.h"
#include <fstream>
#include "..\include\json.hpp"

using json = nlohmann::json;

std::string* Names::getRandomName() {
	if (Names::names.size() == 0) {
		Names::loadRandomNames();
	}

	std::srand(time(0));
	int randomNameIndex = std::rand() % Names::names.size();
	DEBUG(randomNameIndex);
	auto res = Names::names[randomNameIndex];
	Names::names.erase(Names::names.begin() + randomNameIndex);
	return res;
}

void Names::loadRandomNames() {
	std::string file = "";

	if (!names_1) {
		names_1 = true;
		file = "names_1.txt";
	}
	else if (!names_2) {
		names_2 = true;
		file = "names_2.txt";
	}
	else if (!names_3) {
		names_3 = true;
		file = "names_3.txt";
	}
	else if (!names_4) {
		names_4 = true;
		file = "names_4.txt";
	}
	else if (!names_5) {
		names_5 = true;
		file = "names_5.txt";
	}

	try {
		std::ifstream f("res\\names\\" + file);
		std::string content;
		while (std::getline(f, content)) {
			Names::names.push_back(new std::string(content));
		}
	}
	catch (std::exception& e) {
		ERROR("Failed to read " << file);
	}
}

void Names::deloadNames() {
	for (auto name : Names::names) {
		delete name;
	}
	Names::names.clear();
}

std::string TraitGenerator::pickTraitForCategory(const CharacterConfig::Config& cfg, const std::string& persona, const std::string& category) {
	const auto& traits = cfg.categories.at(category);
	const auto& baseDist = (cfg.base.count(category) ? cfg.base.at(category) : std::unordered_map<std::string, double>{});
	const auto& personaMods = (cfg.personas.count(persona) ? cfg.personas.at(persona) : CharacterConfig::Config::PersonaMods{});
	const auto& multDist = (personaMods.mult.count(category) ? personaMods.mult.at(category) : std::unordered_map<std::string, double>{});

	std::vector<double> scores;
	scores.reserve(traits.size());

	for (const auto& trait : traits) {
		double baseW = 1.0;
		if (baseDist.count(trait)) baseW = baseDist.at(trait);
		else if (baseDist.count("*")) baseW = baseDist.at("*");

		double m = (multDist.count(trait) ? multDist.at(trait) : 1.0);

		double score = std::max(cfg.floor, baseW);
		score *= std::pow(m, cfg.persona_mult_weight);

		double n = std::max(-0.99, normalNoise(cfg.add_noise_sigma));
		score *= (1.0 + n);

		scores.push_back(std::max(0.0, score));
	}

	double sum = std::accumulate(scores.begin(), scores.end(), 0.0);
	if (sum <= 0) {
		return traits[uniformIndex(traits.size())];
	}

	double r = dist(rng) * sum;
	for (size_t i = 0; i < traits.size(); i++) {
		r -= scores[i];
		if (r <= 0) return traits[i];
	}
	return traits.back();
}

std::unordered_map<std::string, std::set<std::string>> TraitGenerator::generateCharacterSheet(const CharacterConfig::Config& cfg, const std::string& persona, unsigned int seed) {
	std::mt19937 localRng(seed);
	rng = localRng;

	std::unordered_map<std::string, std::set<std::string>> sheet;
	
	for (const auto& [category, _] : cfg.categories) {
		if (category == "CharacterPersonalities") continue;
		for (int i = 0; i < CharacterConfig::numberOfTraits; i++) {
			sheet[category].insert(pickTraitForCategory(cfg, persona, category));
		}
	}
	return sheet;
}

int TraitGenerator::uniformIndex(size_t n) {
	std::uniform_int_distribution<int> d(0, (int)n - 1);
	return d(rng);
}

double TraitGenerator::normalNoise(double sigma) {
	// Gaussian noise (Box-Muller)
	double u = std::max(1e-12, dist(rng));
	double v = std::max(1e-12, dist(rng));
	return std::sqrt(-2.0 * std::log(u)) * std::cos(2 * 3.14 * v) * sigma;
}

CharacterConfig::Config CharacterConfig::loadConfig(const std::string& filename) {
	std::ifstream file(filename);
	if (!file) throw std::runtime_error("Could not open " + filename);
	json j;
	file >> j;
	CharacterConfig::Config cfg;

	for (auto& [catName, arr] : j["categories"].items()) {
		cfg.categories[catName] = arr.get<std::vector<std::string>>();
	}

	for (auto& [catName, obj] : j["base"].items()) {
		for (auto& [trait, weight] : obj.items()) {
			cfg.base[catName][trait] = weight.get<double>();
		}
	}

	for (auto& [personaName, personaObj] : j["personas"].items()) {
		CharacterConfig::Config::PersonaMods mods;

		if (personaObj.contains("mult")) {
			for (auto& [catName, obj] : personaObj["mult"].items()) {
				for (auto& [trait, weight] : obj.items()) {
					mods.mult[catName][trait] = weight.get<double>();
				}
			}
		}
		cfg.personas[personaName] = std::move(mods);
	}

	if (j.contains("knobs")) {
		cfg.persona_mult_weight = j["knobs"].value("persona_mult_weight", 1.0);
		cfg.add_noise_sigma = j["knobs"].value("add_noise_sigma", 0.05);
		cfg.floor = j["knobs"].value("floor", 0.001);
	}

	return cfg;
}

void CharacterConfig::Config::print() {
	DEBUG("Categories: ");
	for (auto& i : this->categories) {
		DEBUG("\t" << i.first);
		for (auto& j : i.second) {
			DEBUG("\t\t" << j);
		}
	}
	DEBUG("Base: ");
	for (auto& i : this->base) {
		DEBUG("\t" << i.first);
		for (auto& j : i.second) {
			DEBUG("\t\t" << j.first << " " << j.second);
		}
	}

	DEBUG("Mult: ");
	for (auto& i : this->personas) {
		DEBUG("\t" << i.first);
		i.second.print();
	}
}

void CharacterConfig::Config::PersonaMods::print() {
	for (auto& i : this->mult) {
		DEBUG("\t\t" << i.first);
		for (auto& j : i.second) {
			DEBUG("\t\t\t" << j.first << " " << j.second);
		}
	}
}

Character::Character(const std::string& persona, const CharacterConfig::Config& cfg, TraitGenerator* traitGen) {
	this->name = Names::getRandomName();
	this->personality = Traits::stringToPersonalities(persona);
	this->currentEmotions = std::vector<Traits::CharacterEmotions>();
	this->motivations = std::vector<Traits::CharacterMotivations>();
	this->moralities = std::vector<Traits::CharacterMorality>();
	this->intelligence = std::vector<Traits::CharacterIntelligence>();
	this->background = std::vector<Traits::CharacterBackground>();

	auto personality = traitGen->generateCharacterSheet(cfg, persona, time(0));

	for (auto& element : personality) {
		if (element.first == "CharacterBackground") {
			for (auto& e : element.second) {
				this->background.push_back(Traits::stringToBackground(e));
			}
		}
		else if (element.first == "CharacterEmotions") {
			for (auto& e : element.second) {
				this->currentEmotions.push_back(Traits::stringToEmotion(e));
			}
		}
		else if (element.first == "CharacterIntelligence") {
			for (auto& e : element.second) {
				this->intelligence.push_back(Traits::stringToIntelligence(e));
			}
		}
		else if (element.first == "CharacterMorality") {
			for (auto& e : element.second) {
				this->moralities.push_back(Traits::stringToMorality(e));
			}
		}
		else if (element.first == "CharacterMotivations") {
			for (auto& e : element.second) {
				this->motivations.push_back(Traits::stringToMotivation(e));
			}
		}
	}
}

Character::~Character() {
	delete this->name;
}

Traits::CharacterBackground Traits::stringToBackground(const std::string& background) {
	if (background == "WEALTHY") return WEALTHY;
	else if (background == "EDUCATED") return EDUCATED;
	else if (background == "RURAL") return RURAL;
	else if (background == "INDEPENDENT") return INDEPENDENT;
	else if (background == "RELIGIOUS") return RELIGIOUS;
	else if (background == "POOR") return POOR;
	else if (background == "UNEDUCATED") return UNEDUCATED;
	else if (background == "URBAN") return URBAN;
	else if (background == "DEPENDENT") return DEPENDENT;
	else if (background == "SECULAR") return SECULAR;
	else return BACKGROUND_NONE;
}

Traits::CharacterEmotions Traits::stringToEmotion(const std::string& emotion) {
	if (emotion == "ANGRY") return ANGRY;
	else if (emotion == "FINE") return FINE;
	else if (emotion == "RELAXED") return RELAXED;
	else if (emotion == "SAD") return SAD;
	else if (emotion == "CONFUSED") return CONFUSED;
	else if (emotion == "INSPIRED") return INSPIRED;
	else return EMOTION_NONE;
}

Traits::CharacterIntelligence Traits::stringToIntelligence(const std::string& intelligence) {
	if (intelligence == "SMART") return SMART;
	else if (intelligence == "CREATIVE") return CREATIVE;
	else if (intelligence == "SKILLED") return SKILLED;
	else if (intelligence == "STRATEGIC") return STRATEGIC;
	else if (intelligence == "NAIVE") return NAIVE;
	else if (intelligence == "CLUMSY") return CLUMSY;
	else if (intelligence == "RECKLESS") return RECKLESS;
	else return INTELLIGENCE_NONE;
}

Traits::CharacterMorality Traits::stringToMorality(const std::string& morality) {
	if (morality == "LOYAL") return LOYAL;
	else if (morality == "TRUSTING") return TRUSTING;
	else if (morality == "COOPERATIVE") return COOPERATIVE;
	else if (morality == "PROTECTIVE") return PROTECTIVE;
	else if (morality == "CHARMING") return CHARMING;
	else if (morality == "SELFLESS") return SELFLESS;
	else if (morality == "SELFISH") return SELFISH;
	else if (morality == "SUSPICIOUS") return SUSPICIOUS;
	else if (morality == "COMPETETIVE") return COMPETETIVE;
	else if (morality == "NEGLECTFUL") return NEGLECTFUL;
	else if (morality == "AWKWARD") return AWKWARD;
	else return MORALITY_NONE;
}

Traits::CharacterMotivations Traits::stringToMotivation(const std::string& motivation) {
	if (motivation == "POWER") return POWER;
	else if (motivation == "WEALTH") return WEALTH;
	else if (motivation == "LOVE") return LOVE;
	else if (motivation == "KNOWLEDGE") return KNOWLEDGE;
	else if (motivation == "FREEDOM") return FREEDOM;
	else if (motivation == "RESTRICTED") return RESTRICTED;
	else if (motivation == "SAFETY") return SAFETY;
	else if (motivation == "FAIRNESS") return FAIRNESS;
	else return MOTIVATION_NONE;
}

Traits::CharacterPersonalities Traits::stringToPersonalities(const std::string& personality) {
	if (personality == "LEADER") return LEADER;
	else if (personality == "CAREGIVER") return CAREGIVER;
	else if (personality == "THINKER") return THINKER;
	else if (personality == "ADVENTURER") return ADVENTURER;
	else if (personality == "ORGANIZER") return ORGANIZER;
	else if (personality == "PEACEMAKER") return PEACEMAKER;
	else if (personality == "DREAMER") return DREAMER;
	else if (personality == "PERFORMER") return PERFORMER;
	else if (personality == "LOYALIST") return LOYALIST;
	else return PERSONALITY_NONE;
}

std::string Traits::traitToString(CharacterPersonalities personality, CharacterMotivations motivation, CharacterMorality morality, CharacterIntelligence intelligenece, CharacterEmotions emotion, CharacterBackground background) {
	return std::string();
}
