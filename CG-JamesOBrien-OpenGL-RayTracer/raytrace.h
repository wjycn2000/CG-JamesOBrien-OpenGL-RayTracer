#pragma once
#include <glm/glm.hpp>
#include <vector>


struct Triangle {
	glm::vec3 p1, p2, p3;
	glm::vec3 n1, n2, n3;
};

struct Object_encoded {
	glm::vec3 numT; //index, numT, empty
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 color;
	glm::vec3 param1; //kd, ks, shine
	glm::vec3 param2; //km, t, ior
};

struct BVHnode {
	int left, right;
	int index, n;
	glm::vec3 AA, BB;
};

struct BVHnode_encoded {
	glm::vec3 children;
	glm::vec3 triangles;
	glm::vec3 AA, BB;
};

void encodeBVH(const BVHnode& node1, BVHnode_encoded& node2) {
	node2.children = glm::vec3(node1.left, node1.right, 0);
	node2.triangles = glm::vec3(node1.index, node1.n, 0);
	node2.AA = node1.AA;
	node2.BB = node1.BB;
}

void converTree(vector<BVHnode>& tree1, vector<BVHnode_encoded>& tree2) {
	BVHnode_encoded node2;
	for (BVHnode node : tree1) {
		encodeBVH(node, node2);
		tree2.push_back(node2);
	}
}