#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <RayTriangleIntersection.h>
#include <ModelSphere.h>
#include "Image.h"
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <glm/gtc/matrix_access.hpp>
#include <time.h>
#include <random>
#include <omp.h>

using namespace std;
using namespace glm;

#define WIDTH 640
#define HEIGHT 480

#define PI 3.14159265



bool solutionOnTriangle(vec3 i);
Colour getReflection(vector<ModelTriangle> &triangles, vec3 source, vec3 ray, int index);
Colour glass(vector<ModelTriangle>triangles, vec3 ray, vec3 intersection, vec3 normal, int depth);
vec3 getNormal(ModelTriangle triangle);
vec3 getSceneCentre(vector<ModelTriangle> triangles);
vec3 canvasToWorld(CanvasPoint canvasPoint);
vector<Colour> interpolateColour(Colour startColour, Colour endColour, int steps);


void createCoordinateSystem(const vec3 &N, vec3 &Nt, vec3 &Nb);
vec3 uniformSampleHemisphere(const float &r1, const float &r2);
void monteTracer();
bool findRoots(float &a, float &b, float &c, float &t0, float &t1);
vec3 castRay(vec3 &origin, vec3 &direction, int depth, float &absorbFactor, bool insideObject);

struct Intersection {
    vec3 position;
    float distance;
    int triangleIndex;
    int sphereIndex;
};


#define OPAQUE       0
#define TRANSPARENT  1
#define RR_PROB 0.60f
#define SAMPLES 1
#define MIN_DEPTH 4
#define FACTOR 1
DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);


vec3 lightPosition(-0.2334011,4.8,-4.043968);
//for sphere
// vec3 lightPosition(1.5,5,4);
mat3 cameraOrientation(vec3(1.0,0.0,0.0),vec3(0.0,1.0,0.0),vec3(0.0,0.0,1.0));
vec3 cameraPosition(0,1,6);
float focalLength = 250;
//1 = wireframe, 2 = rasterize, 3 = raytrace, 4 = raytrace + AA
int mode = 1;
// 0 = no lighting, 1 = proximity, 2 = specular
int lightingMode = 1;
// 0 = hard shadows, 1 = soft shadows
int shadowMode = 0;
//if 1 face of blue box is mirror
int mirrorBox = 1;
// if 1 red box is glass
int glassBox = 1;
vector<vec3> lightPositions;
vec3 sceneCentre;
int iterations;
int max_depth = 0;

default_random_engine engine(std::random_device{}());
uniform_real_distribution<float> distribution(0.0f, 1.0f);
vec3 image[WIDTH][HEIGHT];
vector<ModelTriangle> cornell;
vector<ModelSphere> spheres;
ModelSphere sphere = ModelSphere(vec3(0.68599, 2.12765,-1.92991), 3.49318f, Colour(150,150,150), 0.0f, OPAQUE, 0.0f, 1.000277f,vec3(0.0f,0.0f, 0.0f));



void update(){
    // if(mode == 3)
    // system("clear");
    // cout << "-------------------------------" << endl;
    // cout << " Iterations:            " << iterations << endl;
    // cout << " Samples / n-th bounce: " << MAX_SAMPLES << " / (" << DROP_FACTOR << " ^ d)" << endl;
    // cout << " Max Depth:             " << max_depth << endl;
    // cout << "-------------------------------" << endl;

    // Function for performing animation (shifting artifacts or moving the camera)
}


//RAYTRACER

bool triangleIntersection(vec3 origin, vec3 direction, ModelTriangle triangle, Intersection &intersection){
    vec3 e0 = vec3(triangle.vertices[1] - triangle.vertices[0]);
    vec3 e1 = vec3(triangle.vertices[2] - triangle.vertices[0]);
    vec3 SPVector = vec3(origin-triangle.vertices[0]);
    mat3 DEMatrix(-direction, e0, e1);
    vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
    intersection.distance = std::numeric_limits<float>::max();
    //  && possibleSolution.x > 0.0001f
    if (solutionOnTriangle(possibleSolution)){
        if (possibleSolution.x < intersection.distance){
            intersection.position = triangle.vertices[0] + possibleSolution.y*e0 + possibleSolution.z*e1;
            intersection.distance = length(origin - intersection.position);
            return true;
        }
    }
    return false;
}
//Analytic method to solve sphereIntersection
bool sphereIntersection(vec3 origin, vec3 direction, ModelSphere sphere, Intersection &intersection){
    float t0, t1;
    vec3  L = origin - sphere.centre;
    float a = dot(direction, direction);
    float b = 2 * dot(direction, L);
    float c = dot(L, L) - (sphere.radius * sphere.radius);
    if (!findRoots(a, b, c, t0, t1)) return false;

    if (t0 > t1) std::swap(t0, t1);
    if (t0 < 0) {
        t0 = t1; // if t0 is negative, let's use t1 instead
        if (t0 < 0) return false; // both t0 and t1 are negative
    }

    intersection.distance = t0;
    intersection.position = origin + direction * t0;
    return true;

}
//CHANGE VARIABLE NAMES
bool findRoots(float &a, float &b, float &c, float &t0, float &t1){
    float d = b * b - 4 * a * c;
    if (d < 0) {
        return false;
    }
    else if (d == 0) {
        t0 = t1 = - 0.5 * b / a;
    }
    else {
        float q = (b > 0) ? -0.5 * (b + sqrt(d)) : -0.5 * (b - sqrt(d));
        t0 = q / a;
        t1 = c / q;
    }
    if (t0 > t1) swap(t0, t1);

    return true;
}

bool getClosestIntersection(vec3 origin, vec3 direction, const vector<ModelTriangle> &triangles, const vector<ModelSphere> &spheres, Intersection &closestIntersection){
    bool found = false;
    float minDistance = std::numeric_limits<float>::max();
    Intersection local;
    for( int i = 0; i < triangles.size(); i++ ) {
        if (triangleIntersection(origin, direction, triangles.at(i), local)) {
            //already checking this
            if (local.distance < minDistance) {
                found = true;
                closestIntersection = local;
                closestIntersection.triangleIndex =  i;
                closestIntersection.sphereIndex   = -99;
                minDistance = local.distance;
            }
        }
    }

    for( int i = 0; i < spheres.size(); i++ ) {
        if (sphereIntersection(origin, direction, spheres.at(i), local)) {
            if (local.distance < minDistance) {
                found = true;
                closestIntersection = local;
                closestIntersection.triangleIndex = -99;
                closestIntersection.sphereIndex   =  i;
                minDistance = local.distance;
            }
        }
    }
    return found;
}
//modify params for fog
// should it return a colour ??
vec3 castRay(vec3 &origin, vec3 &direction, int depth, float &absorbFactor, bool insideObject){
    if(depth > max_depth) max_depth = depth;

    // Russian roulette -- LEARN WHAT THIS DOES PROPERLY
    float prob = 1.0f;
    if (depth > MIN_DEPTH) {
        prob = RR_PROB;
        float r_i = distribution(engine);
        if (r_i < 1.0f - prob) {
            return vec3(0,0,0);
            //return Colour(0,0,0);
        }
    }

    Intersection closestIntersection;
    if(!getClosestIntersection(origin, direction, cornell, spheres, closestIntersection)){
        return vec3(0,0,0);
        //return Colour(0,0,0);
    }

    vec3 normal;
    int   type=0;
    float ior;
    vec3  sigma;
    float emission;
    float reflectivity;
    vec3  surfaceColor;

    float rnd;
    if(closestIntersection.triangleIndex > -1){
        // cout<< "here" << endl;
        ModelTriangle triangle = cornell.at(closestIntersection.triangleIndex);
        normal                 = getNormal(triangle);
        type         = triangle.transparency;
        ior          = triangle.IOR;
        sigma        = triangle.sigma;
        emission     = triangle.emission;
        reflectivity = triangle.reflectivity;
        surfaceColor = vec3(triangle.colour.red, triangle.colour.green, triangle.colour.blue);
        // cout << triangle << endl;
    }
    else if (closestIntersection.sphereIndex > -1) {
        ModelSphere sphere = spheres.at(closestIntersection.sphereIndex);
        normal       = normalize(closestIntersection.position - sphere.centre);
        type         = sphere.transparency;
        ior          = sphere.IOR;
        sigma        = sphere.sigma;
        emission     = sphere.emission;
        reflectivity = sphere.reflectivity;
        surfaceColor = vec3(sphere.colour.red, sphere.colour.green, sphere.colour.blue);
        // cout << sphere << endl;
    }
    // opaque
    vec3 shading;
    if(type == OPAQUE){
        // cout << "here" << endl;
        vec3 N, Nb, Nt;
        N = normal;
        createCoordinateSystem(N, Nt, Nb);
        //direct light
        vec3 emittedLight = emission * vec3(1,1,1);
        //CHMAGE TO SHADOW BIAS
        vec3 hitPosition = closestIntersection.position + normal * 0.00001f;
        // printVec3(hitPosition);
        float fs = 0.0f;
        //TO BE CONTINUED
        if(reflectivity != 0){
            //Shclicks

        }
        rnd = distribution(engine);

        vec3 indirectLight = vec3(0,0,0);
        if(rnd >= fs){
            //for now till we find out what drop factor does
            int samples = std::max( (int) (SAMPLES / pow(FACTOR, depth)), 1 );
            float pdf = 1 / (2* PI);
            for(int i = 0; i < samples; i++ ){
                float r1 = distribution(engine);
                float r2 = distribution(engine);
                vec3 sample = uniformSampleHemisphere(r1, r2);
                // direction
                vec3 sampleWorld = vec3(
                    sample.x * Nb.x + sample.y * N.x + sample.z  *Nt.x,
                    sample.x * Nb.y + sample.y * N.y + sample.z  *Nt.y,
                    sample.x * Nb.z + sample.y * N.z + sample.z  *Nt.z
                );
                // printVec3(sampleWorld);
                indirectLight += r1 * castRay(hitPosition, sampleWorld, depth+1, absorbFactor, insideObject);
                // printVec3(indirectLight);
            }
            indirectLight /= samples * pdf * prob;
            // SURFACE COLOUR IS A COLOUR
            shading = (indirectLight + emittedLight) * surfaceColor / (float) PI;
        }
    }

    //transparent
    return shading;
}

void createCoordinateSystem(const glm::vec3 &N, glm::vec3 &Nt, glm::vec3 &Nb){
    if (std::abs(N.x) > std::abs(N.y)) {
        Nt = vec3(N.z, 0, -N.x)/ sqrtf(N.x * N.x + N.z * N.z);
    } else {
        Nt = vec3(0, -N.z, N.y)/ sqrtf(N.y * N.y + N.z * N.z);
    }
    Nb = cross(N, Nt);
}

vec3 uniformSampleHemisphere(const float &r1, const float &r2){
    float sinTheta = sqrtf(1 - r1 * r1);
    float phi = 2 * PI * r2;
    float x = sinTheta * cosf(phi);
    float z = sinTheta * sinf(phi);
    return vec3(x, r1, z);
}

void monteTracer(){
    iterations +=1;
    cout << "max= " << omp_get_max_threads() << endl;
    #pragma omp parallel for
    for(int y = 0; y < HEIGHT; y++ ){
        for(int x = 0; x < WIDTH; x++){
            id =  omp_get_num_threads();
            float yOffset = distribution(engine) - 0.5f;
            float xOffset = distribution(engine) - 0.5f;
            vec3 point = cameraOrientation * vec3(WIDTH/2.0f -x + xOffset, y-(HEIGHT/2.0f) + yOffset, focalLength);
            // * glm::inverse(cameraOrientation)); ???
            // point = point * glm::inverse(cameraOrientation);
            //FIND OUT WHAT THESE DO
            float holder1;
            // float holder2;

            vec3 colour = castRay(cameraPosition, point, 0, holder1, false);
            // printVec3(colour);
            image[x][y] += colour;
            //convert vec3 to colour
            Colour adjusted = Colour(image[x][y].x /(float)iterations,image[x][y].y /(float) iterations,image[x][y].z /(float) iterations);
            //add corrective filter here
            // cout << adjusted << endl;
            window.setPixelColour(x, y, adjusted.getPacked());
        }
    }
    cout << "threads="<< id << endl;
}

void ScaleTriangles( vector<ModelTriangle>& triangles ) {
    float MAX  = 0.0f;
    float yMax, zMin;
    yMax = numeric_limits<float>::min();
    zMin = numeric_limits<float>::max();
    float scalefactor = 1.1f;
    for( size_t i = 0; i < triangles.size(); ++i ) {
        for(size_t j = 0; j < 3; ++j){
            if (abs(triangles[i].vertices[j].x) > MAX){
                MAX = abs(triangles[i].vertices[j].x);
            }
            else if(abs(triangles[i].vertices[j].y) > MAX){
                MAX = abs(triangles[i].vertices[j].y);
            }
            else if(abs(triangles[i].vertices[j].z) > MAX){
                MAX = abs(triangles[i].vertices[j].z);
            }

            if(triangles[i].vertices[j].y > yMax){
                yMax = triangles[i].vertices[j].y;
            }

            if(triangles[i].vertices[j].z < zMin){
                zMin = triangles[i].vertices[j].z;
            }
        }
    }
    yMax *= 1 / (scalefactor * MAX);
    zMin *= 1 / (scalefactor * MAX);

    for( size_t i = 0; i < triangles.size(); ++i ) {
        // Scale object to fit inside box
        for( size_t j = 0; j < 3; ++j ){
            triangles[i].vertices[j] *= 1 / (scalefactor * MAX);
            // minus ??
            triangles[i].vertices[j].z += (-0.5f - zMin);
            triangles[i].vertices[j].y += (1.0f - yMax);
        }
    	// triangles[i].v0 *= 1 / (SCALE_DOWN * MAX);
    	// triangles[i].v1 *= 1 / (SCALE_DOWN * MAX);
    	// triangles[i].v2 *= 1 / (SCALE_DOWN * MAX);
        //
        // // Move object halfway in front of camera
        // triangles[i].v0.z += (-0.5f - zMin);
    	// triangles[i].v1.z += (-0.5f - zMin);
    	// triangles[i].v2.z += (-0.5f - zMin);
        //
        // // Put object on the floor
        // triangles[i].v0.y += (1.0f - yMax);
    	// triangles[i].v1.y += (1.0f - yMax);
    	// triangles[i].v2.y += (1.0f - yMax);
        //
    	// triangles[i].v0.w = 1.0f;
    	// triangles[i].v1.w = 1.0f;
    	// triangles[i].v2.w = 1.0f;
        //
    	// triangles[i].ComputeNormal();
	}
}



vec3 calculateNormal(ModelTriangle triangle, vec3 possibleSolution){
    vec3 n0 = triangle.normals[0];
    vec3 n1 = triangle.normals[1];
    vec3 n2 = triangle.normals[2];
    vec3 normal = n0 + possibleSolution.y*(n1-n0) +possibleSolution.z*(n2-n0);
    return normal;
}

bool solutionOnTriangle(vec3 i){
    return (0.0<=i.y && i.y<=1.0 && 0.0<=i.z && i.z<=1.0 && (i.y+i.z<=1.0));
}




vec3 getNormal(ModelTriangle triangle){
    vec3 e0 = vec3(triangle.vertices[1] - triangle.vertices[0]);
    vec3 e1 = vec3(triangle.vertices[2] - triangle.vertices[0]);
    vec3 normal = normalize(glm::cross(e0,e1));
    return normal;
}

vec3 findTriangleCentre(ModelTriangle t){
    vec3 sum(0,0,0);
    for(int j = 0; j < 3; j++){
        sum += t.vertices[j];
    }
    sum /= 3.0f;
    return sum;
}

vec3 getSceneCentre(vector<ModelTriangle> triangles){
    vec3 sum(0,0,0);
    for(int i = 0; i < triangles.size(); i++){
        ModelTriangle t = triangles.at(i);
        sum += findTriangleCentre(t);
    }
    sum /= (float)(triangles.size());
    return sum;
}
