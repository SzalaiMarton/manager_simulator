#include "graphics.h"

namespace Graphics {
	sf::Font* Graphics::mainFont = nullptr;
	unsigned int Graphics::currentID = 0;
	std::vector<Graphics::CustomTexture*> Graphics::textures = std::vector<Graphics::CustomTexture*>();
	Renderer* Renderer::instance = nullptr;
	sf::RenderWindow Renderer::window = sf::RenderWindow();
}

Graphics::ObjectBuilder::ObjectBuilder() {
	config = new ObjectConfig();
	config->texture = new sf::Sprite(*Graphics::getTexture(TextureNames::button)); // placeholder texture
}

Graphics::ObjectBuilder& Graphics::ObjectBuilder::withTexture(const std::string& textureName) {
	auto texture = Graphics::getTexture(textureName);
	if (texture) {
		config->texture->setTexture(*texture);
	}
	DEBUG("texture");
	return *this;
}

Graphics::ObjectBuilder& Graphics::ObjectBuilder::withName(const std::string& name) {
	config->name = name;
	DEBUG("name"); 
	return *this;
}

Graphics::ObjectBuilder& Graphics::ObjectBuilder::withHandler(std::function<void()> handler) {
	config->handler = handler;
	DEBUG("zindex"); 
	return *this;
}

Graphics::ObjectBuilder& Graphics::ObjectBuilder::withZIndex(uint8_t renderLayer) {
	config->zIndex = renderLayer;
	return *this;
}

Graphics::ObjectBuilder& Graphics::ObjectBuilder::withLayer(Layer* layer) {
	config->layer = layer;
	DEBUG("layer"); 
	return *this;
}

Graphics::ObjectBuilder& Graphics::ObjectBuilder::withTextureScale(float x, float y) {
	config->texture->setScale({ x, y });
	return *this;
}

Graphics::ObjectBuilder& Graphics::ObjectBuilder::withTexturePos(float x, float y) {
	config->texture->setPosition({ x, y });
	return *this;
}

Graphics::ObjectBuilder& Graphics::ObjectBuilder::withTexturePosOffset(float x, float y) {
	config->texture->setPosition({ config->texture->getPosition().x + x,  config->texture->getPosition().y + y });
	return *this;
}

Graphics::ObjectBuilder& Graphics::ObjectBuilder::withTextureOrigin(float x, float y) {
	config->texture->setOrigin({ config->texture->getGlobalBounds().size.x / x, config->texture->getGlobalBounds().size.y / y });
	return *this;
}

Graphics::ButtonBuilder& Graphics::ButtonBuilder::withText(const std::string& text) {
	config->text = new sf::Text(*Graphics::useMainFont());
	config->text->setString(text);
	config->text->setFillColor(sf::Color::White);
	return *this;
}

Graphics::ButtonBuilder& Graphics::ButtonBuilder::withTextPos(float x, float y) {
	if (config->text) {
		config->text->setPosition({ x, y });
	}
	else {
		ERROR("Text isn't initialized yet. " << config->name);
		throw(std::runtime_error(""));
	}
	return *this;
}

Graphics::ButtonBuilder& Graphics::ButtonBuilder::withTextPosOffest(float x, float y) {
	if (config->text) {
		config->text->setPosition({ config->text->getPosition().x + x, config->text->getPosition().y + y });
	}
	else {
		ERROR("Text isn't initialized yet. " << config->name);
		throw(std::runtime_error(""));
	}
	return *this;
}

Graphics::ButtonBuilder& Graphics::ButtonBuilder::withTextOrigin(float x, float y) {
	if (config->text) {
		config->text->setOrigin({ x, y });
	}
	else {
		ERROR("Text isn't initialized yet. " << config->name);
		throw(std::runtime_error(""));
	}
	return *this;
}

Graphics::ButtonBuilder& Graphics::ButtonBuilder::withTextScale(float x, float y) {
	if (config->text) {
		config->text->setScale({ x, y });
	}
	else {
		ERROR("Text isn't initialized yet. " << config->name);
		throw(std::runtime_error(""));
	}
	return *this;
}

inline Graphics::DropdownElementBuilder& Graphics::DropdownElementBuilder::withParent(DropdownBar* bar) {
	config->parent = bar;
	return *this;
}

Graphics::DropdownElement::DropdownElement(DropdownElementConfig* config) : Button() {
	this->config = config;
}

inline void Graphics::DropdownElement::changeConfig(DropdownElementConfig* config) {
	this->config = config;
}

Graphics::DropdownElement* Graphics::DropdownElementBuilder::build() {
	auto res = new DropdownElement(config);
	if (res->getConfig()->layer) {
		res->getConfig()->layer->addObject(res);
	}
	return res;
}

inline Graphics::DropdownBarBuilder& Graphics::DropdownBarBuilder::withContents(const std::vector<std::string>& texts) {
	config->contents.reserve(texts.size());
	auto elementBuilder = DropdownElementBuilder();
	for (auto& t : texts) {
		elementBuilder.withText(t).withTexture(TextureNames::button);
		config->contents.emplace_back(elementBuilder.build());
	}
}

inline Graphics::DropdownBar* Graphics::DropdownBarBuilder::build() {
	auto res = new DropdownBar(config);
	if (res->getConfig()->layer) {
		res->getConfig()->layer->addObject(res);
	}
	return res;
}

inline Graphics::InputBox* Graphics::InputBoxBuilder::build() {
	auto res = new InputBox(config);
	if (res->getConfig()->layer) {
		res->getConfig()->layer->addObject(res);
	}
	DEBUG("build");
	return res;
}

Graphics::Background* Graphics::BackgroundBuilder::build() {
	Background* res = new Background(config);
	if (res->getConfig()->layer) {
		res->getConfig()->layer->addObject(res);
	}
	DEBUG("build"); 
	return res;
}

Graphics::Button* Graphics::ButtonBuilder::build() {
	auto res = new Button(config);
	if (res->getConfig()->layer) {
		res->getConfig()->layer->addObject(res);
	}
	DEBUG("build");
	return res;
}

void Graphics::Layer::onRender(sf::RenderWindow& window) {
	for (auto& objs : this->objs) {
		if (objs.second.empty()) {
			continue;
		}
		for (auto& element : objs.second) {
			try {
				element->render(&window);
			}
			catch (std::exception& e) {
				ERROR("Faild to render texture. " << e.what());
			}
		}
	}

	if (!this->continuousRendering) {
		this->alreadyRendered = true;
	}
}

Graphics::Renderer::Renderer() : innerLayerCount(20), outerLayerCount(10) {
	this->prevTargetObject = nullptr;
	this->currentTargetObject = nullptr;
	this->currentClickedObject = nullptr;
	this->prevClickedObject = nullptr;
	this->inputBoxActive = false;

	this->window = sf::RenderWindow(sf::VideoMode({ LayoutDesign::width, LayoutDesign::height }), "manager");
	this->currentGameState = MAIN_MENU;
}

Graphics::Renderer::~Renderer() {
}

void Graphics::Renderer::refreshFrame() {
	this->window.clear();
	for (auto& layer : this->onScreenLayers) {
		layer.second->onRender(window);
	}
	this->window.display();
}

void Graphics::Renderer::hideLayer(Layer* layer) {
	for (auto it = this->onScreenLayers.begin(); it != this->onScreenLayers.end(); it++) {
		if ((*it).second == layer) {
			this->onScreenLayers.erase(it);
			return;
		}
	}
}

void Graphics::Renderer::hideLayer(const std::string& name) {
	for (auto it = this->onScreenLayers.begin(); it != this->onScreenLayers.end(); it++) {
		if ((*it).second->name == name) {
			this->onScreenLayers.erase(it);
			return;
		}
	}

	DEBUG("onscreen elements:");
	for (auto& e : this->onScreenLayers) {
		DEBUG(e.second->name);
		for (auto& l : e.second->objs) {
			for (auto& o : l.second) {
				DEBUG("\t" << o->getConfig()->name);
			}
		}
	}
}

void Graphics::Renderer::revealLayer(Layer* layer) {
	this->onScreenLayers.insert({ layer->outerRenderLayer, layer });
}

void Graphics::Renderer::revealLayer(const std::string& name) {
	for (auto& e : this->layers) {
		if (e->name == name) {
			this->onScreenLayers.insert({e->outerRenderLayer, e});
		}
	}
}

sf::Texture* Graphics::getTexture(const std::string& name) {
	std::string fileName = name + ".png";
	if (Graphics::textures.empty()) {
		LOG("Loading textures...");
		Graphics::loadTextures();
		LOG("Finished loading.");
	}

	for (auto& texture : Graphics::textures) {
		if (texture->name == fileName)
			return texture->texture;
	}

	return nullptr;
}

Graphics::Object* Graphics::getObjectByCords(float x, float y, std::vector<Graphics::Object*> onScreenObjs) {
	for (auto& element : onScreenObjs) {
		if (element->getConfig()->texture->getPosition().x == x && element->getConfig()->texture->getPosition().y == y) {
			return element;
		}
	}
	return nullptr;
}

void Graphics::loadMainFont() {
	try {
		for (const auto& tex : std::filesystem::directory_iterator("res\\fonts")) {
			if (tex.is_regular_file() && tex.path().filename().string()[0] == 'm' && tex.path().filename().string()[1] == '_') {
				Graphics::mainFont = new sf::Font(tex.path());
				return;
			}
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		ERROR("Error: " << e.what());
	}
}

void Graphics::loadTextures() {  
	Graphics::textures = std::vector<Graphics::CustomTexture*>();
	Graphics::textures.reserve(10);

	try {  
		for (const auto& subfolder : std::filesystem::directory_iterator("res")) {  
			if (subfolder.is_directory() && subfolder.path().filename().string() != "fonts" && subfolder.path().filename().string() != "names") {
				for (const auto& tex : std::filesystem::directory_iterator(subfolder)) {  
					if (tex.is_regular_file()) {  
						sf::Texture* texture = new sf::Texture();  
						if (texture->loadFromFile(tex.path().string())) {  
							Graphics::textures.push_back(new Graphics::CustomTexture(tex.path().filename().string(), texture));
						}
					}  
				}  
			}  
		}  
	}  
	catch (const std::filesystem::filesystem_error& e) {  
		ERROR("Error: " << e.what());
	}  
}

void Graphics::deloadTextures() {
	for (auto tex : Graphics::textures) {
		delete tex;
	}
	Graphics::textures.clear();
}

void Graphics::deloadFont() {
	delete Graphics::mainFont;
}

Graphics::CustomTexture::CustomTexture(const std::string& name, sf::Texture* tex) {
	this->name = name;
	this->texture = tex;
}

Graphics::Object::Object(ObjectConfig* config) {
	this->config = new ObjectConfig(config->texture, config->name, config->handler, config->zIndex, config->layer);
	DEBUG("CONFGI");
}

Graphics::Button::Button(ButtonConfig* config) {
	this->config = config;
}

inline void Graphics::Button::changeConfig(ButtonConfig* config) {
	this->config = config;
}

inline void Graphics::InputBox::changeConfig(InputBoxConfig* config) {
	this->config = config;
}

inline Graphics::InputBoxConfig* Graphics::InputBox::getConfig() {
	return this->config;
}

inline Graphics::DropdownElementConfig* Graphics::DropdownElement::getConfig() {
	return this->config;
}

inline void Graphics::DropdownBar::changeConfig(DropdownBarConfig* config) {
	this->config = config;
}

inline Graphics::DropdownBarConfig* Graphics::DropdownBar::getConfig() {
	return this->config;
}

inline Graphics::ButtonConfig* Graphics::Button::getConfig() {
	return this->config;
}

inline Graphics::BackgroundConfig* Graphics::Background::getConfig() {
	return this->config;
}

inline Graphics::ObjectConfig* Graphics::Object::getConfig() {
	return this->config;
}

inline void Graphics::Button::hide() {
	this->config->texture->setColor(sf::Color(255, 255, 255, 0));
	this->config->text->setFillColor(sf::Color(255, 255, 255, 0));
}

inline void Graphics::Button::reveal() {
	this->config->texture->setColor(sf::Color(255, 255, 255, 255));
	this->config->text->setFillColor(sf::Color(255, 255, 255, 255));
}

void Graphics::Button::onPress() {
	if (this->config->handler) {
		this->config->handler();
	}
	else {
		ERROR("No handler bind to object.");
	}
}

void Graphics::Background::render(sf::RenderWindow* window) {
	if (this->config->texture && this->isVisible()) {
		window->draw(*this->config->texture);
	}
}

void Graphics::Button::render(sf::RenderWindow* window) {
	if (this->config->texture && this->config->text && this->isVisible()) {
		window->draw(*this->config->texture);
		window->draw(*this->config->text);
	}
}

inline bool Graphics::Object::isVisible() const {
	return (this->config->texture->getColor().a != (uint8_t)0);
}

inline bool Graphics::Object::contains(float x, float y) const {
	if (this->config->texture) {
		return this->config->texture->getGlobalBounds().contains({ x,y });
	}
	return false;
}

void Graphics::InputBox::render(sf::RenderWindow* window) {
	if (this->config->texture && this->config->text && this->config->description && this->isVisible()) {
		window->draw(*this->config->texture);
		window->draw(*this->config->text);
		window->draw(*this->config->description);
	}
}

void Graphics::DropdownBar::render(sf::RenderWindow* window) {
	if (this->config->texture && this->config->text && this->config->description && this->isVisible()) {
		window->draw(*this->config->texture);
		window->draw(*this->config->text);
		window->draw(*this->config->description);
		for (auto& e : this->config->contents) {
			e->render(window);
		}
	}
}

inline void Graphics::Button::changePos(float x, float y) {
	this->config->texture->setPosition({ x,y });
	this->config->text->setPosition({ x,y });
}

inline void Graphics::Button::move(float x, float y) {
	this->config->texture->move({ x,y });
	this->config->text->move({ x,y });
}

inline void Graphics::Object::hide() {
	this->config->texture->setColor(sf::Color(255, 255, 255, 0));
}

inline void Graphics::Object::reveal() {
	this->config->texture->setColor(sf::Color(255, 255, 255, 255));
}

inline void Graphics::Object::onHover() {
	this->config->texture->setColor(sf::Color(0, 0, 0, 128));
}

inline void Graphics::Object::onHoverLoss() {
	this->config->texture->setColor(sf::Color(255, 255, 255, 255));
}

inline void Graphics::Object::changePos(float x, float y) {
	this->config->texture->setPosition({ x,y });
}

inline void Graphics::Object::move(float x, float y) {
	this->config->texture->move({ x,y });
}

sf::Font* Graphics::useMainFont() {
	if (!Graphics::mainFont)
		Graphics::loadMainFont();

	return Graphics::mainFont;
}

Graphics::Background::Background(BackgroundConfig* config) : Object(config) {
	this->config = config;
}

Graphics::InputBox::InputBox(InputBoxConfig* config) : Button(config) {
	this->config = config;
}

void Graphics::InputBox::changeText(const char& newChar, bool removeChar) {
	if (removeChar) {
	this->config->description->setFillColor(sf::Color::Black);
		this->config->text->setString( this->config->text->getString().substring(0, this->config->text->getString().getSize() - 1) );
		if (this->config->text->getString().getSize() > 1) {
			this->config->text->setPosition(this->config->texture->getPosition());
		}
	}
	else {
		this->config->text->setString((this->config->text->getString() + sf::String(newChar)).toUtf32());
		if (this->config->text->getString().getSize() > 1) {
			this->config->text->setPosition(this->config->texture->getPosition());
		}
	}
	this->config->text->setOrigin({ this->config->text->getGlobalBounds().size.x / 2.f, this->config->text->getOrigin().y });
}

void Graphics::InputBox::onPress() {
	this->config->text->setString("");
	this->config->text->setOrigin({ this->config->text->getGlobalBounds().size.x / 2.f, this->config->text->getOrigin().y });
	this->config->text->setPosition(this->config->texture->getPosition());
}

void Graphics::InputBox::onFocusLoss() {
	if (this->config->text->getString() == "") {
		this->config->text->setString("Input");
		this->config->text->setOrigin({ this->config->text->getGlobalBounds().size.x / 2.f, this->config->text->getOrigin().y });
		this->config->text->setPosition(this->config->texture->getPosition());
	}
}

Graphics::DropdownBar::DropdownBar(DropdownBarConfig* config) : Button() {
	this->config = config;
}

Graphics::DropdownBar::~DropdownBar() {
	for (auto e : this->config->contents) {
		delete e;
	}
	delete this->config->text;
	delete this->config->description;
}

void Graphics::DropdownBar::onFocusLoss() {
	for (auto& e : this->config->contents) {
		e->hide();
	}
}

void Graphics::DropdownBar::onPress() {
	if (!this->config->contents.empty()) {
		for (int i = 0; i < this->config->contents.size(); i++) {
			this->config->contents[i]->setPosition(this->config->contents[i]->getConfig()->parent->getConfig()->texture->getGlobalBounds().size.x, this->config->texture->getGlobalBounds().size.y * (i + 1));
			this->config->contents[i]->reveal();
			DEBUG(this->config->contents[i]->getConfig()->name << " revealed");
		}
	}
	else {
		ERROR("No content has been added.");
	}
}

void Graphics::Renderer::handleHover(std::optional<sf::Event> event) {
	if (event->is<sf::Event::MouseMoved>()) {
		this->currentTargetObject = this->getTargetObject((sf::Vector2f)sf::Mouse::getPosition(this->window), &this->onScreenLayers);
		
		if (this->prevTargetObject != this->currentTargetObject) {

			if (this->prevTargetObject) {
				this->prevTargetObject->onHoverLoss();
			}
			if (!dynamic_cast<Graphics::Background*>(this->currentTargetObject) && this->currentTargetObject) {
				this->currentTargetObject->onHover();
			}

			this->prevTargetObject = this->currentTargetObject;
		}
	}
}

void Graphics::Renderer::handleClick(std::optional<sf::Event> event) {
	if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
		this->currentClickedObject = this->getTargetObject((sf::Vector2f)sf::Mouse::getPosition(this->window), &this->onScreenLayers);

		// focus loss
		if (this->currentClickedObject != this->prevClickedObject && this->prevClickedObject) {
			this->prevClickedObject->onFocusLoss();
			if (dynamic_cast<Graphics::InputBox*>(this->prevClickedObject)) {
				this->inputBoxActive = false;
			}
		}

		// clicked object
		if (mouseButtonPressed->button == sf::Mouse::Button::Left && this->currentClickedObject && this->currentClickedObject != this->prevClickedObject) {
			this->currentClickedObject->onPress();
			if (dynamic_cast<Graphics::InputBox*>(this->currentClickedObject)) {
				this->inputBoxActive = true;
			}
		}
		this->prevClickedObject = this->currentClickedObject;
	}
}

void Graphics::Renderer::handleKeyPress(std::optional<sf::Event> event) {
	if (this->inputBoxActive && event->getIf<sf::Event::TextEntered>()) {
		const auto* text = event->getIf<sf::Event::TextEntered>();
		if (dynamic_cast<Graphics::InputBox*>(this->currentClickedObject) && text->unicode != 8) {
			dynamic_cast<Graphics::InputBox*>(this->currentClickedObject)->changeText(text->unicode);
		}
		else {
			dynamic_cast<Graphics::InputBox*>(this->currentClickedObject)->changeText({}, true);
		}
	}
}

void Graphics::Renderer::handleScroll(std::optional<sf::Event> event) {
	if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
		switch (mouseWheelScrolled->wheel) {
		case sf::Mouse::Wheel::Vertical:
			std::cout << "wheel type: vertical" << std::endl;
			break;
		case sf::Mouse::Wheel::Horizontal:
			std::cout << "wheel type: horizontal" << std::endl;
			break;
		}
	}
}

Graphics::Object* Graphics::Renderer::getTargetObject(const sf::Vector2f& mousePos, std::map<uint8_t, Graphics::Layer*>* list) {
	for (auto& outerLayers : *list) {
		for (size_t layerIT = outerLayers.second->objs.size(); layerIT != -1; layerIT--) {
			for (auto& it : outerLayers.second->objs[layerIT]) {
				if (it->contains(mousePos.x, mousePos.y) && it->isVisible()) {
					return it;
				}
			}
		}
	}
	return nullptr;
}

Graphics::Object* Graphics::Renderer::getTargetObject(const std::string& name, std::map<uint8_t, Graphics::Layer*>* list) {
	for (auto& outerLayers : *list) {
		for (auto& layer : outerLayers.second->objs) {
			for (auto it = layer.second.end() - 1; it != layer.second.begin(); it--) {
				if ((*it)->getConfig()->name == name) {
					return (*it);
				}
			}
		}
	}
	return nullptr;
}

void Graphics::Layer::addObject(Graphics::Object* obj) {
	if (obj) {
		this->objs[obj->getConfig()->zIndex].emplace_back(obj);
		DEBUG(obj->getConfig()->name << " was added to " << this->name);
	}
}

void Graphics::Layer::print() {
	DEBUG("Printing " << this->name << " layer's elements:");
	for (auto& e : this->objs) {
		for (auto& l : e.second) {
			DEBUG("\t" << l->getConfig()->name);
		}
	}
}

inline void Graphics::Button::setPosition(float x, float y) {
	this->config->texture->setPosition({ x,y });
	this->config->text->setPosition({ x,y });
}

inline void Graphics::Button::setOrigin(float x, float y) {
	this->config->texture->setOrigin({ x, y });
	this->config->text->setOrigin({ x, y });
}