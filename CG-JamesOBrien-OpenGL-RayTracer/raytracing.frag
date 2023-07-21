#version 330 core
#define TRIANGLE_SIZE 6
#define OBJECT_SIZE 7
#define LIGHT_SIZE 2
#define INF 114514.0
#define PI 3.14159265

uniform samplerBuffer triangles;
uniform samplerBuffer objects;
uniform samplerBuffer lights;

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
    // hr.n = normalize(cross(p2p1, p3p1)); 
    normalize(hr.n);
    // if(dot(hr.n, r.d) > 0){
    //     hr.n = -hr.n;
    //     hr.isInside = true;
    // }
    return hr;
};

HitRecord trace(Ray r, float t0, float t1){
    HitRecord closest;
    HitRecord hitrecord;
    Triangle tr;
    for (int i = 0; i < objectNum; i++) {
        Object o = getObject(i);
        int first = int(o.numT.x);
        int last = first + int(o.numT.y);
        for(int k = first; k < last; k++){
            tr  = getTriangle(k, i);    
            hitrecord = intersect(tr, r, t0, t1);
            if(hitrecord.isHit){
                t1 = hitrecord.t;
                closest = hitrecord;
                closest.color = o.color;
                closest.material1 = o.material1;
                closest.material2 = o.material2;
            }
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
        float sx = o.scale.x;
        float sy = o.scale.y;
        float sz = o.scale.z;
        mat4 transformMatrice = mat4(
            sx*cos(ry)*cos(rz), -sy*cos(rx)*sin(rz)+sy*sin(rx)*sin(ry)*cos(rz), sy*sin(rx)*sin(rz)+sy*cos(rx)*sin(ry)*cos(rz), o.position.x,
            sx*cos(ry)*sin(rz), sy*cos(rx)*cos(rz)+sy*sin(rx)*sin(ry)*sin(rz), -sy*sin(rx)*cos(rz)+sy*cos(rx)*sin(ry)*sin(rz), o.position.y,
            -sx*sin(ry), sy*sin(rx)*cos(ry), sy*cos(rx)*cos(ry), o.position.z,
            0, 0, 0, 1.0
        );
        transformMatrices[g] = transpose(transformMatrice);
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





