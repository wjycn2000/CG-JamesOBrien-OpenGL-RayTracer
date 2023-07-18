#pragma once
#include <glm/glm.hpp>

struct Material {
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float kd = 0.75f;
	float ks = 0.25f;
	float shine = 3.5f;
	float km = 0.25f;
	float t = 0.0f;
	float ior = 1.0f;
};

struct Triangle {
	glm::vec3 p1, p2, p3;
	glm::vec3 n1, n2, n3;
};

//struct Triangle_encoded {
//	glm::vec3 p1, p2, p3;
//	glm::vec3 n1, n2, n3;
//	glm::vec3 color;
//	glm::vec3 param1; //kd, ks, shine
//	glm::vec3 param2; //km, t, ior
//};