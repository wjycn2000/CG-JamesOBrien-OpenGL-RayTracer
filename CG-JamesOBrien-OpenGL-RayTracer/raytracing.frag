#version 330 core
#define TRIANGLE_SIZE 6
#define OBJECT_SIZE 7
#define LIGHT_SIZE 2
#define NODE_SIZE 4
#define INF 114514.0
#define PI 3.14159265

uniform samplerBuffer triangles;
uniform samplerBuffer objects;
uniform samplerBuffer lights;
uniform samplerBuffer trees;

out vec4 FragColor;

uniform vec3 eye;
uniform vec3 at;
uniform vec3 up;
uniform float hither;
uniform float shadowbias;
uniform float angle;
uniform ivec2 resolution;
uniform vec3 bcolor;
uniform int triangleNum;
uniform int objectNum;
uniform int lightNum;
mat4[10] transformMatrices;


struct Ray {
    vec3 e;
    vec3 d;
    int depth;
};

struct Triangle {
    vec3 p1, p2, p3;
	vec3 n1, n2, n3;
};

struct Object {
    vec3 numT;
    vec3 position;
    vec3 rotation;
    vec3 scale;
    vec3 color;
    vec3 material1; //kd, ks, shine
    vec3 material2; //km, t, ior
};

struct Light {
    vec3 position;
    vec3 color;
};

struct BVHnode{
    vec3 AA, BB;
    int left, right;
    int index, n;
};

struct HitRecord {
    int raydepth;
    bool isHit;
    bool isInside;
    float t;
    vec3 p;
    vec3 n;
    vec3 v;
    vec3 color;
    vec3 material1; //kd, ks, shine
    vec3 material2; //km, t, ior
};

Object getObject(int i) {
    int offset = i * OBJECT_SIZE;
    Object o;

    o.numT = texelFetch(objects, offset+0).xyz;
    o.position = texelFetch(objects, offset+1).xyz;
    o.rotation = texelFetch(objects, offset+2).xyz;
    o.scale = texelFetch(objects, offset+3).xyz;
    o.color = texelFetch(objects, offset+4).xyz;
    o.material1 = texelFetch(objects, offset+5).xyz;
    o.material2 = texelFetch(objects, offset+6).xyz;

    
    return o;
}

BVHnode getBVH(int i) {
    int offset = i * NODE_SIZE;
    BVHnode node;

    vec3 children = vec3(texelFetch(trees, offset).xyz);
    vec3 leaf = vec3(texelFetch(trees, offset+1).xyz);
    node.left = int(children.x);
    node.right = int(children.y);
    node.index = int(leaf.x);
    node.n = int(leaf.y);
    node.AA = texelFetch(trees, offset+2).xyz;
    node.BB = texelFetch(trees, offset+3).xyz;   
    return node;
}

Object getObjectOfTriangle(int pos){
    Object o;
    for(int k = 0; k < objectNum; k++){  
        o = getObject(k); 
        if(pos>=o.numT.x && pos < o.numT.x + o.numT.y){
            break;
        }
    }
    return o;
}

int getObjectIndexOfTriangle(int pos){
    Object o;
    int k;
    for(k = 0; k < objectNum; k++){  
        o = getObject(k); 
        if(pos>=o.numT.x && pos < o.numT.x + o.numT.y){
            break;
        }
    }
    return k;
}

Triangle getTriangle(int i, int objectIndex) {
    int offset = i * TRIANGLE_SIZE;
    Triangle t;

    mat4 transformMatrix = transformMatrices[objectIndex];

    t.p1 = (transformMatrix * vec4(texelFetch(triangles, offset + 0).xyz, 1.0f)).xyz;
    t.p2 = (transformMatrix * vec4(texelFetch(triangles, offset + 1).xyz, 1.0f)).xyz;
    t.p3 = (transformMatrix * vec4(texelFetch(triangles, offset + 2).xyz, 1.0f)).xyz;
    t.n1 = (transformMatrix * vec4(texelFetch(triangles, offset + 3).xyz, 0)).xyz;
    t.n2 = (transformMatrix * vec4(texelFetch(triangles, offset + 4).xyz, 0)).xyz;
    t.n3 = (transformMatrix * vec4(texelFetch(triangles, offset + 5).xyz, 0)).xyz;

    return t;
};

Light getLight(int i) {
    int offset = i * LIGHT_SIZE;
    Light l;

    l.position = texelFetch(lights, offset).xyz;
    l.color = texelFetch(lights, offset+1).xyz;

    return l;
}

float det(vec3 p1, vec3 p2, vec3 p3) {
    return p1.x * (p2.y * p3.z - p3.y * p2.z) + p2.x * (p3.y * p1.z - p1.y * p3.z) + p3.x * (p1.y * p2.z - p2.y * p1.z);
};

BVHnode getNewAABB(BVHnode node, int objectIndex){
    BVHnode newNode = node;
    mat4 transformMatrix = transformMatrices[objectIndex];
    vec3 vertexes[8];
    vertexes[0] = vec3(node.AA.x, node.AA.y, node.AA.z);
    vertexes[1] = vec3(node.AA.x, node.BB.y, node.AA.z);
    vertexes[2] = vec3(node.AA.x, node.AA.y, node.BB.z);
    vertexes[3] = vec3(node.AA.x, node.BB.y, node.BB.z);
    vertexes[4] = vec3(node.BB.x, node.AA.y, node.AA.z);
    vertexes[5] = vec3(node.BB.x, node.AA.y, node.BB.z);
    vertexes[6] = vec3(node.BB.x, node.BB.y, node.AA.z);
    vertexes[7] = vec3(node.BB.x, node.BB.y, node.BB.z);
    float minx = INF, maxx = -INF, miny = INF, maxy = -INF, minz = INF, maxz = -INF;
    for(int i =0; i < 8; i++){
        vertexes[i] = (transformMatrix * vec4(vertexes[i], 1.0f)).xyz;
    }
    for(int i = 0; i < 8; i++){
        if(vertexes[i].x < minx) minx = vertexes[i].x;
        if(vertexes[i].x > maxx) maxx = vertexes[i].x;
        if(vertexes[i].y < miny) miny = vertexes[i].y;
        if(vertexes[i].y > maxy) maxy = vertexes[i].y;
        if(vertexes[i].z < minz) minz = vertexes[i].z;
        if(vertexes[i].z > maxz) maxz = vertexes[i].z;
    }
    newNode.AA = vec3(minx, miny, minz);
    newNode.BB = vec3(maxx, maxy, maxz);
    return newNode;
}

bool hitAABB(Ray r, BVHnode node, int objectIndex){
    float txmin, txmax, tymin, tymax, tzmin, tzmax;
    float a;
    node = getNewAABB(node, objectIndex);
    //x-direction
    a = 1/r.d.x;
    if(a>=0){
        txmin = a*(node.AA.x - r.e.x);
        txmax = a*(node.BB.x - r.e.x);
    }
    else{
        txmin = a*(node.BB.x - r.e.x);
        txmax = a*(node.AA.x - r.e.x);
    }
    //y-direction
    a = 1/r.d.y;
    if(a>=0){
        tymin = a*(node.AA.y - r.e.y);
        tymax = a*(node.BB.y - r.e.y);
    }
    else{
        tymin = a*(node.BB.y - r.e.y);
        tymax = a*(node.AA.y - r.e.y);
    }
    //z-direction
    a = 1/r.d.z;
    if(a>=0){
        tzmin = a*(node.AA.z - r.e.z);
        tzmax = a*(node.BB.z - r.e.z);
    }
    else{
        tzmin = a*(node.BB.z - r.e.z);
        tzmax = a*(node.AA.z - r.e.z);
    }
    //see if they have overlap
    if(txmin>tymax || tymin>txmax || txmin>tzmax || tzmin>txmax || tzmin>tymax || tymin>tzmax){
        return false;
    }else{
        return true;
    }
}

HitRecord intersect(Triangle tr, Ray r, float t0, float t1) { 
    HitRecord hr;
    hr.isHit = false;
    hr.isInside = false;
    vec3 p2p1 = tr.p1 - tr.p2;
    vec3 p3p1 = tr.p1 - tr.p3;
    vec3 ep1 = tr.p1 - r.e;
    float detA = det(p2p1, p3p1, r.d);
    float t = det(p2p1, p3p1, ep1) / detA;
    if (t < t0 || t > t1) return hr;
    float beta = det(ep1, p3p1, r.d) / detA;
    if (beta < 0 || beta > 1) return hr;
    float gamma = det(p2p1, ep1, r.d) / detA;
    if (gamma < 0.0 || gamma > 1.0 - beta) return hr;
    hr.isHit = true;
    hr.t = t;
    hr.p = r.e + t * r.d;
    hr.v = -r.d;
    hr.raydepth = r.depth;
    float alpha = 1.0f - beta - gamma;
    hr.n = alpha * tr.n1 + beta * tr.n2 + gamma * tr.n3;
    normalize(hr.n);
    return hr;
};

HitRecord hitTriangles(Ray r, float t0, float t1, int left, int right, int objectIndex){
    HitRecord hr;
    hr.isHit = false;
    hr.t = INF;
    for(int i=left;i<=right;i++){
        Triangle tr = getTriangle(i, objectIndex);
        HitRecord hitrecord = intersect(tr, r, t0, t1);
        if(hitrecord.isHit){
            t1 = hitrecord.t;
            hr = hitrecord;
        }
    }
    return hr;
};

HitRecord hitBVH(Ray r, float t0, float t1, int objectIndex){
    HitRecord hr;
    hr.isHit=false;
    hr.t=INF;

    Object o = getObject(objectIndex);
    int root = int(o.numT.x);

    // debug
    // int end = root + int(o.numT.y);
    // for(int i = root; i<end;i++){
    //     BVHnode node = getBVH(i);
    //     if(node.n>0){
    //         HitRecord hitRecord = hitTriangles(r, t0, t1, node.index, node.index+node.n-1, objectIndex);
    //         if(hitRecord.isHit && hitRecord.t<hr.t){
    //             hr=hitRecord;
    //         }
    //     }
    // }
    // return hr;

    int stack[256];
    int sp=0;

    stack[sp++] = root;
    while(sp>0){
        int top = stack[--sp];
        BVHnode node = getBVH(top);

        //leaf node
        if(node.n>0){
            int left = node.index;
            int right = node.index + node.n - 1;
            HitRecord hrt = hitTriangles(r, t0, t1, left, right, objectIndex);
            if(hrt.isHit && hrt.t<hr.t){
                hr = hrt;
            }
            continue;
        }

        BVHnode leftChild = getBVH(node.left);
        bool leftHit = hitAABB(r, leftChild, objectIndex);
        BVHnode rightChild = getBVH(node.right);
        bool rightHit = hitAABB(r, rightChild, objectIndex);
        if(leftHit) stack[sp++] = node.left;
        if(rightHit) stack[sp++] = node.right;
    }

    return hr;
};

HitRecord trace(Ray r, float t0, float t1){
    HitRecord closest;
    closest.isHit=false;
    closest.t=INF;
    // HitRecord hitrecord;
    // Triangle tr;
    for (int i = 0; i < objectNum; i++) {
        Object o = getObject(i);
        HitRecord hit = hitBVH(r, t0, t1, i);
        if(hit.isHit && hit.t<closest.t){
            closest = hit;
            closest.color = o.color;
            closest.material1 = o.material1;
            closest.material2 = o.material2;
        }
    }
    return closest;
}

vec3 shade(HitRecord hr) {  
    if(hr.isHit==false){
        return bcolor;
    }

    vec3 localColor = vec3(0.0f, 0.0f, 0.0f);

    for(int i =0; i < lightNum; i++){
        Light light = getLight(i);
        vec3 l = normalize(light.position - hr.p);

        //shadow test
        bool shadow = false;
        Ray shadowRay;
        shadowRay.e = hr.p;
        shadowRay.d = l;
        HitRecord shadowHit = trace(shadowRay, shadowbias, INF);
        if(shadowHit.isHit){
            continue;
        }

        //diffuse
        float cos = dot(l, hr.n);
        vec3 diffuse = hr.material1.x * max(cos, 0.0f) * light.color;

        //specular
        vec3 reflection = -l + 2 * dot(l, hr.n) * hr.n;
        vec3 specular = hr.material1.y * max(pow(dot(reflection, hr.v), hr.material1.z), 0.0f) * light.color;
    
        localColor += (diffuse + specular) * hr.color;
    }

    
    return localColor;
}

void main()
{
    //add transform matrices
    for(int g = 0; g < objectNum; g++){
        Object o = getObject(g);
        float rx = o.rotation.x * PI / 180.0f;
        float ry = o.rotation.y * PI / 180.0f;
        float rz = o.rotation.z * PI / 180.0f;
        mat4 translation = mat4(
            1, 0, 0, o.position.x,
            0, 1, 0, o.position.y,
            0, 0, 1, o.position.z,
            0, 0, 0, 1
        );
        mat4 rotationZ = mat4(
            cos(rz), -sin(rz), 0, 0,
            sin(rz), cos(rz), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
        mat4 rotationY = mat4(
            cos(ry), 0, sin(ry), 0,
            0, 1, 0, 0,
            -sin(ry), 0, cos(ry), 0,
            0, 0, 0, 1
        );
        mat4 rotationX = mat4(
            1, 0, 0, 0,
            0, cos(rx), -sin(rx), 0,
            0, sin(rx), cos(rx), 0,
            0, 0, 0, 1
        );
        mat4 scaleM = mat4(
            o.scale.x, 0, 0, 0,
            0, o.scale.y, 0, 0,
            0, 0, o.scale.z, 0,
            0, 0, 0, 1
        );
        transformMatrices[g] = transpose(scaleM*rotationZ*rotationY*rotationX*translation);
    } 
    

    //Compute ray
    vec3 w = normalize(eye - at);
    vec3 u = normalize(cross(up, w));
    vec3 v = normalize(cross(w, u));
    float i = gl_FragCoord.x;
    float j = gl_FragCoord.y;
    float d = length(eye - at);
    float verticalHalf = tan((PI / 180) * (angle / 2)) * d;
    float horizontalHalf = resolution.x / resolution.y * verticalHalf;
    float t = verticalHalf;
    float b = -verticalHalf;
    float l = -horizontalHalf;
    float r = horizontalHalf;
    float u1 = l + (r - l)*(i + 0.5)/resolution.x;
    float v1 = b + (t - b)*(j + 0.5)/resolution.y;
    Ray ray;
    ray.e = eye;
    ray.d = normalize(-d*w + u1*u + v1*v);

    HitRecord hr = trace(ray, hither, INF);
    
    vec3 color = shade(hr);

    // 输出最终的颜色
    FragColor = vec4(color, 1.0f);
};





