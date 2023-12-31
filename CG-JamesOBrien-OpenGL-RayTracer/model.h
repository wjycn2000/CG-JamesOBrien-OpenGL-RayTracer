#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include <learnopengl/shader_s.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "raytrace.h"
using namespace std;

struct Material {
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    float kd = 0.75f;
    float ks = 0.25f;
    float shine = 3.5f;
    float km = 0.25f;
    float t = 0.0f;
    float ior = 1.0f;
    Material() {}
    Material(glm::vec3 _color, float _kd, float _ks, float _shine,
        float _km, float _t, float _ior) {
        color = _color;
        kd = _kd;
        ks = _ks;
        shine = _shine;
        km = _km;
        t = _t;
        ior = _ior;
    }
};
class Object;
class Model
{
public:
    // model data 
    vector<Mesh> meshes;
    vector<Object*> objects;
    unsigned int numT;

    // constructor, expects a filepath to a 3D model.
    Model(string const& path)
    {
        loadModel(path);
        numT = 0;
    }

    void genTriangles(vector<Triangle>& triangles) {
        unsigned int numTStart = triangles.size();
        for (unsigned int i = 0; i < meshes.size(); i++) {
            Mesh* mesh = &meshes[i];
            for (unsigned int j = 0; j < mesh->indices.size()-2; j=j+3) {
                Triangle t;
                t.p1 = mesh->vertices[mesh->indices[j]].Position;
                t.p2 = mesh->vertices[mesh->indices[j+1]].Position;
                t.p3 = mesh->vertices[mesh->indices[j+2]].Position;
                t.n1 = mesh->vertices[mesh->indices[j]].Normal;
                t.n2 = mesh->vertices[mesh->indices[j + 1]].Normal;
                t.n3 = mesh->vertices[mesh->indices[j + 2]].Normal;
                triangles.push_back(t);
            }
        }
        numT = triangles.size() - numTStart;
    }

    int genBVHtree(vector<Triangle>& triangles, vector<BVHnode>& bvhnodes, int l, int r, int n) {
        if (l > r) return 0;

        //get index, to access by index
        bvhnodes.push_back(BVHnode());
        int index = bvhnodes.size() - 1;

        //initialize
        bvhnodes[index].left = bvhnodes[index].right = -1;
        bvhnodes[index].n = bvhnodes[index].index = -1;
        bvhnodes[index].AA = glm::vec3(-INFINITY, -INFINITY, -INFINITY);
        bvhnodes[index].BB = glm::vec3(INFINITY, INFINITY, INFINITY);

        //compute AABB
        GLfloat minx = INFINITY, maxx = -INFINITY, miny = INFINITY, maxy=-INFINITY, minz=INFINITY, maxz=-INFINITY;
        for (unsigned int i = l; i <= r; i++) {
            glm::vec3 p1 = triangles[i].p1;
            glm::vec3 p2 = triangles[i].p2;
            glm::vec3 p3 = triangles[i].p3;

            //minx
            if (p1.x < minx) minx = p1.x;
            if (p2.x < minx) minx = p2.x;
            if (p3.x < minx) minx = p3.x;

            //maxx
            if (p1.x > maxx) maxx = p1.x;
            if (p2.x > maxx) maxx = p2.x;
            if (p3.x > maxx) maxx = p3.x;

            //miny
            if (p1.y < miny) miny = p1.y;
            if (p2.y < miny) miny = p2.y;
            if (p3.y < miny) miny = p3.y;

            //maxy
            if (p1.y > maxy) maxy = p1.y;
            if (p2.y > maxy) maxy = p2.y;
            if (p3.y > maxy) maxy = p3.y;

            //minz
            if (p1.z < minz) minz = p1.z;
            if (p2.z < minz) minz = p2.z;
            if (p3.z < minz) minz = p3.z;

            //maxz
            if (p1.z > maxz) maxz = p1.z;
            if (p2.z > maxz) maxz = p2.z;
            if (p3.z > maxz) maxz = p3.z;
        }
        bvhnodes[index].AA = glm::vec3(minx, miny, minz);
        bvhnodes[index].BB = glm::vec3(maxx, maxy, maxz);

        //if too few triangles in a node, create leaf
        if (r - l + 1 <= n) {
            bvhnodes[index].index = l;
            bvhnodes[index].n = r - l + 1;
            return index;
        }



        //sort
        auto begin = std::next(triangles.begin(), l);
        auto end = std::next(triangles.begin(), r);
        float xrange = maxx - minx;
        float yrange = maxy - miny;
        float zrange = maxz - minz;
        float maxrange = std::max({ xrange, yrange, zrange });
        if (maxrange == xrange) {
            sort(begin, end, [](Triangle t1, Triangle t2) {return t1.p1.x < t2.p1.x; });
        }
        else if (maxrange == yrange) {
            sort(begin, end, [](Triangle t1, Triangle t2) {return t1.p1.y < t2.p1.y; });
        }
        else {
            sort(begin, end, [](Triangle t1, Triangle t2) {return t1.p1.z < t2.p1.z; });
        }


        //create child nodes
        int mid = (l + r) / 2;
        int left = genBVHtree(triangles, bvhnodes, l, mid, n);
        int right = genBVHtree(triangles, bvhnodes, mid + 1, r, n);
        bvhnodes[index].left = left;
        bvhnodes[index].right = right;
        return index;
    }

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);

    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            
            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }


        return Mesh(vertices, indices);
    }
};

class Object {
public:
    Model* model;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    Material material;

    Object(Model* _model, Material _m, glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale) {
        model = _model;
        position = _position;
        rotation = _rotation;
        scale = _scale;
        material = _m;
        model->objects.push_back(this);
    }
};
#endif