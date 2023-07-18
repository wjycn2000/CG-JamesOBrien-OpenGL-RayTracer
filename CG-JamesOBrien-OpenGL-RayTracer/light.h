#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "model.h"

struct Light {
	glm::vec3 position;
	glm::vec3 color;
};

class Scene {
public:
	std::vector<Model> models;
	std::vector<Light> lights;
};