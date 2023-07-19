#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "model.h"

struct Light {
	glm::vec3 position;
	glm::vec3 color;

	Light(glm::vec3 _position, glm::vec3 _color = glm::vec3(1.0f)) {
		position = _position;
		color = _color;
	}
};

class Scene {
public:
	std::vector<Object> objects;
	std::vector<Light> lights;
	Scene() {}

	void addObject(Object o) {
		objects.push_back(o);
	}

	void addLight(Light l) {
		lights.push_back(l);
	}

	void generateData(std::vector<Triangle>& triangles, std::vector<Object_encoded>& os, std::vector<Light>& _lights) {
		Object_encoded o;
		for (unsigned int i = 0; i < objects.size(); i++) {
			o.numT.x = triangles.size();
			objects[i].model->genTriangles(triangles);

			o.position = objects[i].position;
			o.rotation = objects[i].rotation;
			o.scale = objects[i].scale;
			Material m = objects[i].material;
			o.color = m.color;
			o.param1 = glm::vec3(m.kd, m.ks, m.shine);
			o.param2 = glm::vec3(m.km, m.t, m.ior);
			o.numT.y = (float)objects[i].model->numT;
			os.push_back(o);
		}
		_lights = lights;
	}
};