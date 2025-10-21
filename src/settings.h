#pragma once

#include <functional>
#include <map>
#include <cstdint>
#include <filesystem>
#include "../include/SFML/Graphics.hpp"
#include "logging.h"
#include "save_creator.h"
#include "graphics.h"
#include "character.h"


constexpr const char* mainMenuLayerName = "MainMenu";
constexpr const char* savesMenuLayerName = "SavesMenu";
constexpr const char* newGameMenuLayerName = "NewGameMenu";