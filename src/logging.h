#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

#if 1
#define LOG(x) std::cout << "[LOG] " << x << std::endl
#else
#define LOG(x) (void*)0
#endif

#if 1
#define DEBUG(x) std::cout << "[DEBUG] " << x << std::endl
#else
#define DEBUG(x) (void*)0
#endif

#if 1
#define ERROR(x) std::cout << "[ERROR] " << x << std::endl
#else
#define ERROR(x) (void*)0
#endif