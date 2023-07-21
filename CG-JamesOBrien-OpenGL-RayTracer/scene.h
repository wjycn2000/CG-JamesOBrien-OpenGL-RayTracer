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

struct Indexes {
	int tindex,tnum;
};

class Scene {
public:
	std::vector<Object> objects;
	std::vector<Light> lights;
	std::vector<Model*> models;
	Scene() {}

	void addObject(Object o) {
		objects.push_back(o);
		auto iter = find(models.begin(), models.end(), o.model);
		if (iter == models.end()) {
			models.push_back(o.model);
		}
	}

	void addLight(Light l) {
		lights.push_back(l);
	}

	void generateData(std::vector<Triangle>& triangles, std::vector<Object_encoded>& os, std::vector<BVHnode>& tree, std::vector<Light>& _lights) {
		vector<bool> model_visited(models.size(), false);
		vector<Indexes> indexes(models.size());
		for (unsigned int i = 0; i < objects.size(); i++) {
			//find the model
			unsigned int j;
			for ( j = 0; j < models.size(); j++) {
				if (models[j] == objects[i].model)
				{
					break;
				}
					
			}
			
			if (!model_visited[j]) {
				model_visited[j] = true;
				int index = triangles.size();
				models[j]->genTriangles(triangles);
				int num = triangles.size() - index;
				int tree_index = tree.size();
				indexes[j].tindex = tree_index;
				models[j]->genBVHtree(triangles, tree, index, index+num-1, 6);
				Object_encoded o;
				o.color = objects[i].material.color;
				o.param1 = glm::vec3(objects[i].material.kd, objects[i].material.ks, objects[i].material.shine);
				o.param2 = glm::vec3(objects[i].material.km, objects[i].material.t, objects[i].material.ior);
				o.position = objects[i].position;
				o.rotation = objects[i].rotation;
				o.scale = objects[i].rotation;
				o.numT.x = tree_index;
				o.numT.y = tree.size() - tree_index;
				indexes[j].tnum = tree.size() - tree_index;
				os.push_back(o);
				tree_index = tree.size();
			}
			else {
				Object_encoded o;
				o.color = objects[i].material.color;
				o.param1 = glm::vec3(objects[i].material.kd, objects[i].material.ks, objects[i].material.shine);
				o.param2 = glm::vec3(objects[i].material.km, objects[i].material.t, objects[i].material.ior);
				o.position = objects[i].position;
				o.rotation = objects[i].rotation;
				o.scale = objects[i].rotation;
				o.numT.x = indexes[j].tindex;
				o.numT.y = indexes[j].tnum;
				os.push_back(o);
			}
		}

		_lights = lights;
	}
};