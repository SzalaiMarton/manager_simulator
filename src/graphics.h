#pragma once

#include "settings.h"

namespace LayoutDesign {
	constexpr uint8_t top = 0;
	constexpr uint8_t left = 0;
	constexpr uint8_t bot = 1;
	constexpr uint8_t right = 1;
	constexpr float middle = 0.5f;

	constexpr uint16_t width = 800;
	constexpr uint16_t height = width * (16/9);

	constexpr float center_x = width / 2;
	constexpr float center_y = height / 2;
	constexpr float footer_y = height * 0.9f;
	constexpr float header_y = height * 0.1f;

	constexpr uint8_t global_scaleX = 4;
	constexpr uint8_t global_scaleY = 6;

	constexpr uint8_t description_size = 25;
}

namespace TextureNames {
	const std::string button = "button";
	const std::string dropdownBar = "dropdown_bar";
	const std::string inputBar = "input_bar";
	const std::string mainBG = "main_background_single_color";
}

namespace Graphics {
	class Layer;
	class DropdownElement;
	class DropdownBar;

	struct ObjectConfig {
		sf::Sprite* texture;
		std::string name;
		std::function<void()> handler;
		uint8_t zIndex;
		Layer* layer;

		ObjectConfig() : zIndex(1), layer(nullptr), handler(nullptr), name("sadsadasdsa"), texture(nullptr) { DEBUG("obj config"); }
		ObjectConfig(sf::Sprite* t, const std::string n, std::function<void()> h, uint8_t zI, Layer* l) :
			texture(t), name(n), handler(h), zIndex(zI), layer(l) {}
	};

	struct BackgroundConfig : public ObjectConfig {
		BackgroundConfig() : ObjectConfig() { DEBUG("background config" << name); }
	};

	struct ButtonConfig : public ObjectConfig {
		sf::Text* text;
		sf::Text* description;

		ButtonConfig() : ObjectConfig(), text(nullptr), description(nullptr) { DEBUG("button config"); }
	};

	struct InputBoxConfig : public ButtonConfig {

		InputBoxConfig() : ButtonConfig() {}
	};

	struct DropdownElementConfig : public ButtonConfig {
		DropdownBar* parent;

		DropdownElementConfig() : ButtonConfig(), parent(nullptr) {}
	};

	struct DropdownBarConfig : public ButtonConfig {
		std::vector<DropdownElement*> contents;

		DropdownBarConfig() : ButtonConfig(), contents({}) {}
	};


	class CustomTexture {
	public:
		std::string name;
		sf::Texture* texture;

		CustomTexture(const std::string& name, sf::Texture* tex);
		~CustomTexture() = default;
	};

	class Object {
	private:
		ObjectConfig* config;
	public:

		Object() { config = new ObjectConfig(); }
		Object(ObjectConfig* config);
		~Object() = default;

		inline virtual ObjectConfig* getConfig();
		inline virtual void onPress() = 0;
		inline virtual void onFocusLoss() = 0;
		inline virtual void render(sf::RenderWindow*) = 0;
		inline bool isVisible() const;
		inline bool contains(float x, float y) const;
		inline void hide();
		inline void reveal();
		inline void onHover();
		inline void onHoverLoss();
		inline void changePos(float x, float y);
		inline void move(float x, float y);
	};

	class Button : public Object {
	private:
		ButtonConfig* config;
	public:

		Button() : Object() {}
		Button(ButtonConfig* config);
		~Button() = default;

		inline void changeConfig(ButtonConfig* config);
		inline ButtonConfig* getConfig();

		void onPress() override;
		void onFocusLoss() override {};
		void render(sf::RenderWindow* window) override;
		inline void changePos(float x, float y);
		inline void move(float x, float y);
		inline void hide();
		inline void reveal();
		inline void setPosition(float x, float y);
		inline void setOrigin(float x, float y);
	};

	class Background : public Object {
	private:
		BackgroundConfig* config;
	public:
		Background(BackgroundConfig* config);
		~Background() = default;

		inline BackgroundConfig* getConfig() override;
		inline void onPress() override {}
		inline void onFocusLoss() override {};
		void render(sf::RenderWindow* window) override;
		inline void onHover() {}
		inline void onHoverLoss() {}
	};

	class InputBox : public Button {
	private:
		InputBoxConfig* config;
	public:

		InputBox(InputBoxConfig* config);
		~InputBox() = default;
	
		inline void changeConfig(InputBoxConfig* config);
		inline InputBoxConfig* getConfig() override;
		void render(sf::RenderWindow* window) override;
		void onPress() override;
		void onFocusLoss() override;
		void changeText(const char& newChar = {}, bool removeChar = false);
	};

	class DropdownElement : public Button {
	private:
		DropdownElementConfig* config;
	public:

		DropdownElement(DropdownElementConfig* config);
		
		inline void changeConfig(DropdownElementConfig* config);
		inline DropdownElementConfig* getConfig() override;
		inline void render(sf::RenderWindow*) override {};
		inline void onFocusLoss() override {};
	};

	class DropdownBar : public Button {
	private:
		enum Direction {
			UP = -1,
			DOWN = 1
		};
		DropdownBarConfig* config;

	public:
		DropdownBar(DropdownBarConfig* config);
		~DropdownBar();

		inline void changeConfig(DropdownBarConfig* config);
		inline DropdownBarConfig* getConfig();

		void render(sf::RenderWindow* window) override;
		void onPress() override;
		void onFocusLoss() override;
		//void onScroll(bool up);
		//void createContentList(const std::vector<std::string>& contentList);
		//void moveContents(Direction direction);
	};

	class ObjectBuilder {
	private:
		ObjectConfig* config;
	public:

		ObjectBuilder();
		inline virtual ObjectBuilder& withTexture(const std::string& textureName);
		inline virtual ObjectBuilder& withName(const std::string& name);
		inline virtual ObjectBuilder& withHandler(std::function<void()> handler);
		inline virtual ObjectBuilder& withZIndex(uint8_t renderLayer);
		inline virtual ObjectBuilder& withLayer(Layer* layer);
		inline virtual ObjectBuilder& withTextureScale(float x, float y);
		inline virtual ObjectBuilder& withTexturePos(float x, float y);
		inline virtual ObjectBuilder& withTexturePosOffset(float x, float y);
		inline virtual ObjectBuilder& withTextureOrigin(float x, float y);
		inline virtual Object* build() = 0;
	};

	class BackgroundBuilder : public ObjectBuilder {
	private:
		BackgroundConfig* config;
	public:

		BackgroundBuilder() : ObjectBuilder() { config = new BackgroundConfig(); }
		Background* build() override;
	};

	class ButtonBuilder : public ObjectBuilder {
	private:
		ButtonConfig* config;
	public:

		ButtonBuilder() : ObjectBuilder() { config = new ButtonConfig(); }
		inline ButtonBuilder& withText(const std::string& text);
		inline ButtonBuilder& withTextPos(float x, float y);
		inline ButtonBuilder& withTextPosOffest(float x, float y);
		inline ButtonBuilder& withTextOrigin(float x, float y);
		inline ButtonBuilder& withTextScale(float x, float y);
		Button* build() override;
	};

	class DropdownElementBuilder : public ButtonBuilder {
	private:
		DropdownElementConfig* config;
	public:

		DropdownElementBuilder() : ButtonBuilder() { config = new DropdownElementConfig(); };
		inline DropdownElementBuilder& withParent(DropdownBar* bar);
		DropdownElement* build() override;
	};

	class DropdownBarBuilder : public ButtonBuilder {
	private:
		DropdownBarConfig* config;
	public:

		DropdownBarBuilder() : ButtonBuilder() { config = new DropdownBarConfig(); };
		inline DropdownBarBuilder& withContents(const std::vector<std::string>& texts);
		DropdownBar* build() override;
	};

	class InputBoxBuilder : public ButtonBuilder {
	private:
		InputBoxConfig* config;
	public:

		InputBoxBuilder() : ButtonBuilder() { config = new InputBoxConfig(); }
		InputBox* build() override;
	};


	class Layer {
	public:
		std::string name;
		bool continuousRendering;
		bool alreadyRendered;
		uint8_t outerRenderLayer;
		std::map<uint8_t, std::vector<Graphics::Object*>> objs;

		Layer() = default;
		Layer(const std::string& name, bool continuousRendering, uint8_t outerRenderLayer) : name(name), continuousRendering(continuousRendering), alreadyRendered(false), outerRenderLayer(outerRenderLayer) 
		{ objs = std::map<uint8_t, std::vector<Graphics::Object*>>(); }

		void onRender(sf::RenderWindow& window);

		void addObject(Graphics::Object* obj);
		void print();
	};

	class Renderer {
	private:
		static Renderer* instance;

		Renderer();
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
	public:
		enum States {
			MAIN_MENU,
			NEW_GAME_MENU,
			SAVES_MENU,
			SETTINGS_MAIN,
			IN_GAME
		};

		static sf::RenderWindow window;
		Graphics::Renderer::States currentGameState;
		
		Graphics::Object* currentTargetObject;
		Graphics::Object* prevTargetObject;
		Graphics::Object* currentClickedObject;
		Graphics::Object* prevClickedObject;
		
		bool inputBoxActive;
		const uint8_t innerLayerCount;
		const uint8_t outerLayerCount;
		
		std::map<uint8_t, Layer*> onScreenLayers;
		std::vector<Layer*> layers;

		static Renderer* getRender() {
			if (!instance) {
				instance = new Renderer();
			}
			return instance;
		}

		void refreshFrame();
		void hideLayer(Layer* layer);
		void hideLayer(const std::string& name);
		void revealLayer(Layer* layer);
		void revealLayer(const std::string&name);
		void addLayer(Layer* layer);
		void createLayer(const std::string& name, bool constRefresh, uint8_t outerRenderLayer);
		Layer* getLayer(const std::string& name);

		void handleHover(std::optional<sf::Event> event);
		void handleClick(std::optional<sf::Event> event);
		void handleKeyPress(std::optional<sf::Event> event);
		void handleScroll(std::optional<sf::Event> event);

		Graphics::Object* getTargetObject(const sf::Vector2f& mousePos, std::map<uint8_t, Graphics::Layer*>* list);
		Graphics::Object* getTargetObject(const std::string& name, std::map<uint8_t, Graphics::Layer*>* list);

		void deleteVector(const std::map<uint8_t, std::vector<Graphics::Object*>>& vector);
	};

	sf::Texture* getTexture(const std::string& name);
	Graphics::Object* getObjectByCords(float x, float y, std::vector<Graphics::Object*> onScreenObjs);
	sf::Font* useMainFont();
	void loadMainFont();
	void loadTextures();

	void deloadTextures();
	void deloadFont();

	extern sf::Font* mainFont;
	extern std::vector<Graphics::CustomTexture*> textures;
	extern unsigned int currentID;
}