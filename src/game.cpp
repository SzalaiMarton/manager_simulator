#include "game.h"

Game::Game::Game() {
	//this->saves = std::vector<SaveCreator::Save*>();
	this->renderer = Graphics::Renderer::getRender();

	LOG("Loading textures...");
	Graphics::loadTextures();
	LOG("Finished loading.");

	LOG("Loading names...");
	Names::loadRandomNames();
	LOG("Finished loading");

	this->characterConfig = CharacterConfig::loadConfig("res\\traits.json");
	this->traitGenerator = TraitGenerator((unsigned)time(0));

	this->renderer->createLayer(mainMenuLayerName, false, 1);
	this->renderer->createLayer(savesMenuLayerName, false, 1);
	this->renderer->createLayer(newGameMenuLayerName, false, 2);

	Graphics::BackgroundBuilder builder = Graphics::BackgroundBuilder();
	builder.withTexture(TextureNames::mainBG)
		.withLayer(this->renderer->getLayer(mainMenuLayerName))
		.withName("mainBG")
		.withZIndex(1);
	this->mainMenuBG = builder.build();

	//this->initMainMenu();
	//this->initNewGameMenu();
	//this->initSavesMenu();
	
	this->renderer->revealLayer(mainMenuLayerName);
}

void Game::Game::initMainMenu() {
	Graphics::ButtonBuilder newGameBuilderButton = Graphics::ButtonBuilder();
	newGameBuilderButton.withName("New Game")
		.withTexture(TextureNames::button)
		.withLayer(this->renderer->getLayer(mainMenuLayerName))
		.withHandler([this]() { this->renderer->hideLayer(mainMenuLayerName); this->renderer->revealLayer(newGameMenuLayerName); })
		.withZIndex(2)
		.withTexturePos(LayoutDesign::center_x, LayoutDesign::center_y)
		.withTextureOrigin(2.f, 2.f)
		.withTexturePosOffset(0, 400)
		.withTextureScale(LayoutDesign::global_scaleX, LayoutDesign::global_scaleY);
	auto newGameButton = newGameBuilderButton.build();
}

void Game::Game::initSavesMenu() {
	/*new Graphics::Button({LayoutDesign::center_x, LayoutDesign::footer_y}, {0, 0},
		{ LayoutDesign::global_scaleY, LayoutDesign::global_scaleX }, "Back", 
		[this]() { this->renderer->revealLayer(mainMenuLayerName); this->renderer->hideLayer(savesMenuLayerName); }, 
		2, this->renderer->getLayer(savesMenuLayerName));
	*/
	
	this->renderer->getLayer(savesMenuLayerName)->addObject(this->mainMenuBG);
}

void Game::Game::initNewGameMenu() {
	/*new Graphics::InputBox({LayoutDesign::center_x, 100}, {0, 0},
		{ LayoutDesign::global_scaleY, LayoutDesign::global_scaleX }, 
		"New_Game_Name_Input", "Name", nullptr, 2, this->renderer->getLayer(newGameMenuLayerName));
	new Graphics::DropdownBar({ LayoutDesign::center_x, 100 }, { 0, 100 }, 
		{ LayoutDesign::global_scaleY, LayoutDesign::global_scaleX }, 
		"Outer_Mode_Selector", "Mode", {"ASd", "asd"}, nullptr, 2, this->renderer->getLayer(newGameMenuLayerName));
	new Graphics::DropdownBar({ LayoutDesign::center_x, 100 }, { 0, 200 }, 
		{ LayoutDesign::global_scaleY, LayoutDesign::global_scaleX }, 
		"Inner_Mode_Selector", "Style", {"asd", "asd"}, nullptr, 2, this->renderer->getLayer(newGameMenuLayerName));
	new Graphics::Button({ LayoutDesign::center_x, 100 }, { 0, 300 }, 
		{ LayoutDesign::global_scaleY, LayoutDesign::global_scaleX }, "Back",
		[this]() { this->renderer->revealLayer(mainMenuLayerName); this->renderer->hideLayer(newGameMenuLayerName); }, 
		2, this->renderer->getLayer(newGameMenuLayerName));
	*/

	this->renderer->getLayer(newGameMenuLayerName)->addObject(this->mainMenuBG);
}

void Game::Game::run() {
	while (this->renderer->window.isOpen()) {
		while (std::optional event = this->renderer->window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				this->shutdown();
				return;
			}
			
			this->renderer->handleHover(event);
			this->renderer->handleClick(event);
			this->renderer->handleKeyPress(event);

			this->renderer->refreshFrame();
		}
	}
}

void Game::Game::shutdown() {
	this->renderer->window.close();
	Graphics::deloadTextures();
	Graphics::deloadFont();
	delete this->mainMenuBG;
	delete this->onScreenBG;
	exit(0);
}

void Graphics::Renderer::addLayer(Layer* layer) {
	this->layers.emplace_back(layer);
}

void Graphics::Renderer::createLayer(const std::string& name, bool constRefresh, uint8_t outerRenderLayer) {
	this->layers.emplace_back(new Layer(name, constRefresh, outerRenderLayer));
}

Graphics::Layer* Graphics::Renderer::getLayer(const std::string& name) {
	for (auto& l : layers) {
		if (l->name == name) {
			return l;
		}
	}
	return nullptr;
}


void Graphics::Renderer::deleteVector(const std::map<uint8_t, std::vector<Graphics::Object*>>& vector) {
	for (auto& layer : vector) {
		for (auto element : layer.second) {
			delete element;
		}
	}
}
