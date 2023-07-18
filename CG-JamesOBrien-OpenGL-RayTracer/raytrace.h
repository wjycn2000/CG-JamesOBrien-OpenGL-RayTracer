#pragma once
#include <glm/glm.hpp>


struct Triangle {
	glm::vec3 p1, p2, p3;
	glm::vec3 n1, n2, n3;
};

struct Object_encoded {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 color;
	glm::vec3 param1; //kd, ks, shine
	glm::vec3 param2; //km, t, ior
};

//struct Triangle_encoded {
//	glm::vec3 p1, p2, p3;
//	glm::vec3 n1, n2, n3;
//	glm::vec3 color;
//	glm::vec3 param1; //kd, ks, shine
//	glm::vec3 param2; //km, t, ior
//};