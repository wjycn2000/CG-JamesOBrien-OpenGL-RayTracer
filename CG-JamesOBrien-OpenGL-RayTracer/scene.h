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
	std::vector<Model*> models;
	Scene() {}

	void addObject(Object o) {
		objects.push_back(o);
	}

	void addLight(Light l) {
		lights.push_back(l);
	}

	void generateData(std::vector<Triangle>& triangles, std::vector<Object_encoded>& os, std::vector<Light>& _lights) {
		//add models
		for (unsigned int i = 0; i < objects.size(); i++) {
			auto iter = std::find(models.begin(), models.end(), objects[i].model);
			if (iter == models.end()) {
				models.push_back(objects[i].model);
			}

		}
		int index = 0;
		for (unsigned int i = 0; i < models.size(); i++) {
			Model* model = models[i];
			model->genTriangles(triangles);
			for (unsigned int j = 0; j < model->objects.size(); j++) {
				Object_encoded o;

				o.numT.x = index;
				o.numT.y = triangles.size() - index;

				o.position = model->objects[j]->position;
				o.rotation = model->objects[j]->rotation;
				o.scale = model->objects[j]->scale;
				Material m = model->objects[j]->material;
				o.color = m.color;
				o.param1 = glm::vec3(m.kd, m.ks, m.shine);
				o.param2 = glm::vec3(m.km, m.t, m.ior);
				os.push_back(o);
			}
			index = triangles.size();
		}
		_lights = lights;
	}
};