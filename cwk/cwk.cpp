#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <RayTriangleIntersection.h>
#include "Image.h"
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include "threeDPoint.h"
#include <glm/gtc/matrix_access.hpp>


using namespace std;
using namespace glm;

#define WIDTH 700
#define HEIGHT 600

#define PI 3.14159265

void draw();
void update();
void handleEvent(SDL_Event event);
void drawLine(CanvasPoint from, CanvasPoint to, Colour colour);
void drawStrokeTriangle(CanvasTriangle t);
void drawRandomTriangle(bool filled);
void drawFilledTriangle(CanvasTriangle t, vector<float> &zArray);
Image loadImage();
void savePPM();
void createTextureTriangle();
vector<float> getEmptyZArray();
vector<ModelTriangle> loadObj(string path);
void drawRow(CanvasPoint from, CanvasPoint to, Colour colour, vector<float> &zArray);
void rasterizeModel(vector<ModelTriangle> triangles);
void printMat3(mat3 m);
void printVec3(vec3 v);
void printMat4(mat4 m);
void printVec4(vec4 v);
void draw(vector<ModelTriangle> model);
void raytraceModel(vector<ModelTriangle> model);
void raytraceModelAA(vector<ModelTriangle> triangles);
void drawWireframes(vector<ModelTriangle> triangles);
void initialiseLights(vector<ModelTriangle>triangles, int numberOfLights);


DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

//taken from light position in obj file
vec3 lightPosition(-0.2334011,4,-3.043968);
mat3 cameraOrientation(vec3(1.0,0.0,0.0),vec3(0.0,1.0,0.0),vec3(0.0,0.0,1.0));
vec3 cameraPosition(0,1,6);
float focalLength = 250;
//1 = wireframe, 2 = rasterize, 3 = raytrace, 4 = raytrace + AA
int mode = 1;
// 0 = no lighting, 1 = proximity, 2 = specular
int lightingMode = 1;
// 0 = hard shadows, 1 = soft shadows
int shadowMode = 1;
vector<vec3> lightPositions;

int main(int argc, char* argv[])
{
    // vector<ModelTriangle> model = loadObj("cornell-box/cornell-box.obj");
    vector<ModelTriangle> model = loadObj("logo/logo.obj");
    initialiseLights(model, 10);
    for(int i=0; i<lightPositions.size(); i++){
        // solution for now
        lightPositions.at(i).y = lightPositions.at(i).y - 0.5f;
    }
    SDL_Event event;
    while(true)
    {
        // We MUST poll for events - otherwise the window will freeze !
        if(window.pollForInputEvents(&event)) {
            handleEvent(event);
        }
        update();
        draw(model);

    }
}

void draw(vector<ModelTriangle> model){
    if (mode != 0){
        window.clearPixels();
        if (mode==1){
            drawWireframes(model);

        } else if(mode==2){
            rasterizeModel(model);

        } else if(mode==3){
            raytraceModel(model);
            cout << "Model raytraced, waiting for user enter" << endl;
            mode = 0;
        }
        else if(mode==4){
            raytraceModelAA(model);
            cout << "AA Model raytraced, waiting for user enter" << endl;
            mode = 0;
        }
        window.renderFrame();
    }
}

void update(){


    // Function for performing animation (shifting artifacts or moving the camera)
}


void drawLine(CanvasPoint from, CanvasPoint to, Colour colour){
    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
    float xStepSize = xDiff/numberOfSteps;
    float yStepSize = yDiff/numberOfSteps;
    for (float i=0.0; i<numberOfSteps; i++) {
        float x = from.x + (xStepSize*i);
        float y = from.y + (yStepSize*i);
        window.setPixelColour(round(x), round(y), colour.getPacked());
    }
}
//**** Helper functions for Wu line algorithm *****
//integer part of x
float ipart(float x){
    return floor(x);
}

float roundPart(float x){
    return ipart(x+0.5);
}

//fractional part of x
float fpart(float x){
    return x-floor(x);
}

float rfpart(float x){
    return 1 - fpart(x);
}

void drawPoint(int x, int y, Colour colour, float brightness){
    Colour adjustedColour = Colour(colour.red, colour.green, colour.blue, brightness);
    window.setPixelColour(x, y, adjustedColour.getPacked());
}


void drawWuLine(CanvasPoint from, CanvasPoint to, Colour colour){
    // bool steep = ifSteep(from, to);
    float x0 = from.x;
    float y0 = from.y;
    float x1 = to.x;
    float y1 = to.y;
    int steep = abs(y1-y0) > abs(x1-x0);

    if(steep){
        swap(x0, y0);
        swap(x1, y1);
    }
    if(x0 > x1){
        swap(x0, x1);
        swap(y0, y1);

    }
    //compute the slope
    float dx = x1-x0;
    float dy = y1-y0;
    float gradient = dy/dx;
    if(dx == 0.0){
        gradient = 1;
    }
    //first endpoint
    int xend = round(x0);
    float yend = y0 + (gradient * (xend-x0));
    float xgap = fpart(x0+0.5);
    int xpxl1 = xend;
    int ypxl1 = ipart(yend);

    if(steep){
        drawPoint(ypxl1, xpxl1, colour, rfpart(yend)*xgap);
        drawPoint(ypxl1+1, xpxl1, colour, fpart(yend)*xgap);
    }
    else{
        drawPoint(xpxl1, ypxl1  , colour, rfpart(yend) * xgap);
        drawPoint(xpxl1, ypxl1+1, colour, fpart(yend) * xgap);
    }
    float yintersect =  yend + gradient; // first y intersection

    //second enpoint
    xend = round(x1);
    yend = y1 + (gradient *(xend - x1));
    xgap = fpart(x1 +0.5);
    int xpxl2 = xend;
    int ypxl2 = ipart(yend);

    if(steep){
        drawPoint(ypxl2, xpxl2, colour, rfpart(yend) * xgap);
        drawPoint(ypxl2+1, xpxl2, colour, fpart(yend) * xgap);
    }
    else{
        drawPoint(xpxl2, ypxl2, colour, rfpart(yend) * xgap);
        drawPoint(xpxl2, ypxl2+1, colour, fpart(yend) * xgap);
    }

    // main loop
    if(steep){
        for(int x=xpxl1+1; x<xpxl2-1; x++){
            drawPoint(ipart(yintersect), x, colour, rfpart(yintersect));
            drawPoint(ipart(yintersect)+1, x, colour, fpart(yintersect));
            yintersect = yintersect + gradient;
        }
    }
    else{
        for(int x=xpxl1+1; x<xpxl2-1; x++){
            drawPoint(x, ipart(yintersect), colour, rfpart(yintersect));
            drawPoint(x, ipart(yintersect)+1, colour, fpart(yintersect));
            yintersect = yintersect + gradient;
        }
    }
}

void drawStrokeTriangle(CanvasTriangle t){
    drawLine(t.vertices[0], t.vertices[1], t.colour);
    drawLine(t.vertices[0], t.vertices[2], t.colour);
    drawLine(t.vertices[1], t.vertices[2], t.colour);
}

void drawWuStrokeTriangle(CanvasTriangle t){
    drawWuLine(t.vertices[0], t.vertices[1], t.colour);
    drawWuLine(t.vertices[0], t.vertices[2], t.colour);
    drawWuLine(t.vertices[1], t.vertices[2], t.colour);
}


void drawFrame(CanvasTriangle t, vector<float> &zArray){
    drawRow(t.vertices[0], t.vertices[1], t.colour, zArray);
    drawRow(t.vertices[0], t.vertices[2], t.colour, zArray);
    drawRow(t.vertices[1], t.vertices[2], t.colour, zArray);
}

void drawFilledTriangle(CanvasTriangle t, vector<float> &zArray){
    t.calculateTriangleMeta();
    // cout << t.topStarts.size() << endl;
    for (int i = 0; i < t.topStarts.size(); i++){
        drawRow(t.topStarts.at(i), t.topEnds.at(i), t.colour, zArray);
    }
    for (int i = 0; i < t.bottomStarts.size(); i++){
        drawRow(t.bottomStarts.at(i), t.bottomEnds.at(i), t.colour, zArray);
    }
}

void drawRow(CanvasPoint from, CanvasPoint to, Colour colour, vector<float> &zArray){
    if (from.x > to.x) swap(to, from);
    int y = round(from.y);
    int rowWidth = round(to.x-from.x);
    for (int x = 0; x < rowWidth; x++){
        if ((y < HEIGHT) && (y >= 0)  && ((from.x+x) >=0) && ((from.x+x) < WIDTH-1) ){
            if (rowWidth == 1) rowWidth = 2;
            float depth = -1/(from.depth+((to.depth-from.depth)*x/(rowWidth-1)));
            if (depth > zArray.at(y*WIDTH + from.x + x)){
                zArray.at(y*WIDTH + from.x + x) = depth;
                window.setPixelColour(round(from.x + x), y, colour.getPacked());
            } else {
                // cout << "z array: " << zArray.at(y*WIDTH + from.x + x) << " depth: " << depth << endl;
                // cout << "pixel is behind " << from.x+x << ",  " << y << endl;
                // window.setPixelColour(round(from.x + x), y, Colour(255, 255, 255).getPacked());
            }
            // cout << "z array: " << zArray.at(y*WIDTH + from.x + x) << " depth: " << depth << endl;
        }
    }
}

vector<CanvasPoint> interpolateRow(CanvasPoint start, CanvasPoint end, int steps){
    vector<CanvasPoint> points;
    for (int i=0; i<steps; i++){
        CanvasPoint p;
        if (steps == 1) steps = 2;
        p.x = start.x+((end.x-start.x)*i/(steps-1));
        p.y = start.y+((end.y-start.y)*i/(steps-1));
        p.depth = start.depth+((end.depth-start.depth)*i/(steps-1));
        points.push_back(p);
    }
    return points;
}


Image loadImage(){
    string pSix;
    string data;
    string widthAndHeight;
    string colourRange;
    ifstream file;
    file.open("texture.ppm");
    getline(file, pSix);
    getline(file,data);
    getline(file, widthAndHeight);
    getline(file, colourRange);
    int spacePosition = widthAndHeight.find(" ");
    int newlinePosition = widthAndHeight.find("\n");
    int width = stoi(widthAndHeight.substr(0,spacePosition));
    int height = stoi(widthAndHeight.substr(spacePosition,newlinePosition));
    vector<Colour> pixels;
    int numOfPixels = width * height;
    int red;
    int green;
    int blue;
    for (size_t i = 0; i < numOfPixels; i++ ){
        red = file.get();
        green = file.get();
        blue = file.get();
        Colour pixel = Colour(red, green, blue);
        pixels.push_back(pixel);
    }
    Image i = Image(width, height, pixels);
    return i;
}

void savePPM(){
    char red;
    char green;
    char blue;
    uint32_t pixel;
    //add to_string(n) for different frame numbers
    string filename = "./frames/frame.ppm";
    ofstream out(filename, std::ios::out | std::ios::binary);
    if(!out){
        std::cerr << "Cannot open:" << filename << endl;
        exit(1);
    }
    out << "P6\n";
    out << "# Created by zackdrei\n";
	out << to_string(WIDTH) << " " << to_string(HEIGHT) << "\n";
	out << "255\n";
    for(int y=0; y<HEIGHT; y++){
        for(int x=0; x<WIDTH; x++){
            pixel = window.getPixelColour(x,y);
            red = char((pixel >> 16 ) & 255);
			green = char((pixel >> 8) & 255);
			blue = char((pixel & 255));
			out << red << green << blue;
        }
    }
    out.close();
    cout << "saved" << endl;
}

void displayImage(){
    Image image = loadImage();
    for (int y=0; y<image.height; y++){
        for (int x=0; x<image.width; x++){
            window.setPixelColour(x, y, image.pixels.at((y*image.width)+x).getPacked());
        }
    }
}


vector<CanvasPoint> interpolatePoints(CanvasPoint start, CanvasPoint end, int steps){
    vector<CanvasPoint> points;
    for (int i=0; i<steps; i++){
        CanvasPoint p;
        if (steps == 1) steps = 2;
        p.x = start.x+((end.x-start.x)*i/(steps-1));
        p.y = start.y+((end.y-start.y)*i/(steps-1));
        // cout << "R " << v.x << endl;
        // cout << "G " << v.y << endl;
        // cout << "B " << v.z << endl;
        points.push_back(p);
    }
    return points;
}

CanvasPoint texturePointToCanvasPoint(TexturePoint t){
    CanvasPoint p = CanvasPoint(t.x, t.y);
    return p;
}

void createTextureTriangle(){
    Image i = loadImage();
    CanvasPoint a = CanvasPoint(160, 10);
    TexturePoint ap = TexturePoint(195, 5);
    a.texturePoint = ap;
    CanvasPoint b = CanvasPoint(300, 230);
    TexturePoint bp = TexturePoint(395, 380);
    b.texturePoint = bp;
    CanvasPoint c = CanvasPoint(10, 150);
    TexturePoint cp = TexturePoint(65 ,330);
    c.texturePoint = cp;
    CanvasTriangle t = CanvasTriangle(a, b, c);
    t.calculateTriangleMeta();
    float ratio = (t.middle.y - t.top.y) / (t.bottom.y - t.top.y);
    t.middleIntersect.texturePoint.x = (t.bottom.texturePoint.x - t.top.texturePoint.x) * ratio + t.top.texturePoint.x;
    t.middleIntersect.texturePoint.y = (t.bottom.texturePoint.y - t.top.texturePoint.y) * ratio + t.top.texturePoint.y;
    // drawTrianglesForTexture(t);
    //Top triangle
    int height = t.middle.y-t.top.y;
    vector<CanvasPoint> starts = interpolatePoints(t.top, t.middle, height);
    vector<CanvasPoint> ends = interpolatePoints(t.top, t.middleIntersect, height);
    vector<CanvasPoint> textureStarts = interpolatePoints(texturePointToCanvasPoint(t.top.texturePoint), texturePointToCanvasPoint(t.middle.texturePoint), height);
    vector<CanvasPoint> textureEnds = interpolatePoints(texturePointToCanvasPoint(t.top.texturePoint), texturePointToCanvasPoint(t.middleIntersect.texturePoint), height);
    for (int y = 0; y<height; y++){
        int width = ends.at(y).x - starts.at(y).x;
        // cout << width << endl;
        vector<CanvasPoint> row = interpolatePoints(textureStarts.at(y), textureEnds.at(y), width);
        for (int x=0; x<width; x++){
            window.setPixelColour(starts.at(y).x+x, starts.at(y).y, i.getPixel(row.at(x).x, row.at(x).y).getPacked());
        }
    }
    cout << "starting bottom triangle" << endl;
    //Bottom triangle
    int bottomHeight = t.bottom.y-t.middle.y;
    vector<CanvasPoint> bottomStarts = interpolatePoints(t.middle, t.bottom, bottomHeight);
    vector<CanvasPoint> bottomEnds = interpolatePoints(t.middleIntersect, t.bottom, bottomHeight);
    // cout << bottomStarts.size() << endl;
    // cout << bottomEnds.size() << endl;
    vector<CanvasPoint> bottomTextureStarts = interpolatePoints(texturePointToCanvasPoint(t.middle.texturePoint), texturePointToCanvasPoint(t.bottom.texturePoint), bottomHeight);
    vector<CanvasPoint> bottomTextureEnds = interpolatePoints(texturePointToCanvasPoint(t.middleIntersect.texturePoint), texturePointToCanvasPoint(t.bottom.texturePoint), bottomHeight);
    for (int y = 0; y<bottomHeight; y++){
        int width = bottomEnds.at(y).x - bottomStarts.at(y).x;
        // cout << width << endl;
        // cout << y << endl;
        vector<CanvasPoint> row = interpolatePoints(bottomTextureStarts.at(y), bottomTextureEnds.at(y), width);
        for (int x=0; x<width; x++){
            cout << "x: " << x << endl;
            window.setPixelColour(bottomStarts.at(y).x+x, bottomStarts.at(y).y-1, i.getPixel(row.at(x).x, row.at(x).y).getPacked());
        }
    }
}

vector<Colour> loadMtl(string path){
    string nameLine;
    string propertiesLine;
    ifstream file(path, std::ios::in);
    // file.open(path);
    // if (file.fail()){
    //     cout << ".MTL FILE COULD NOT BE OPENED" << endl;
    // }
    if(!file){
        std::cerr << "Cannot open " << path << std::endl;
		exit(1);
    }
    vector<Colour> colours;
    while (true) {
        getline(file, nameLine);
        //Char 32 is space
        char space = char(32);
        string name = split(nameLine, space)[1];
        // cout << name << endl;
        getline(file, propertiesLine);
        string redString = split(propertiesLine, space)[1];
        string greenString = split(propertiesLine, space)[2];
        string blueString = split(propertiesLine, space)[3];
        Colour c = Colour(name, redString, greenString, blueString);
        colours.push_back(c);
        // cout << c << endl;
        string emptyLine;
        getline(file, emptyLine);
        if( file.eof() ) break;
    }
    return colours;
}

Colour getColourByName(vector<Colour> colours, string name){
    for (int i = 0; i<colours.size(); i ++){
        if (!name.compare(colours.at(i).name)){
            return colours.at(i);
        }
    }
    cout << "Colour not found" << endl;
    return Colour();
}

vector<ModelTriangle> loadObj(string path){
    ifstream file;
    file.open(path);
    char space = char(32);
    vector<string> materialFileNames;
    vector<vec3> points;
    vector<Colour> colours;
    vector<ModelTriangle> triangles;
    vector<TexturePoint> texturePoints;
    //Get vertices, and material files
    while (true){
        string line;
        getline(file, line);
        string* items = split(line, space);
        if (!items[0].compare("mtllib")){
            cout<< items[1] << endl;
            //change this to read in from logo.obj
            vector<Colour> newColours = loadMtl(items[1]);
            colours.insert(colours.end(), newColours.begin(), newColours.end() );
        } else if (!items[0].compare("v")){
            vec3 point = vec3( stof(items[1]), stof(items[2]), stof(items[3]));
            // vec3 point = vec3(1, 1, 1);
            points.push_back(point);
        }
        else if(!items[0].compare("vt")){
            TexturePoint point = TexturePoint(stof(items[1]), stof(items[2]));
            texturePoints.push_back(point);
        }
        if (file.eof() ) break;
    }
    cout << "Vertices and material files loaded" << endl;
    //Get triangles, and their objects & materials, reset file pointer first
    file.clear();
    file.seekg(0, ios::beg);
    string objectName;
    while (true){
        string line;
        getline(file, line);
        string* items = split(line, space);
        string materialName;
        Colour currentColour;
        if (!items[0].compare("o")){
            objectName = items[1];
        }
        if (!items[0].compare("usemtl")){
            materialName = items[1];
            currentColour = getColourByName(colours, materialName);
        }
        if (!items[0].compare("f")){
            // number after / represents indices for texture points
            cout << "1: " << items[1] << "2: " << items[2] << "3: " << items[3] << endl;
            items[1].pop_back();
            items[2].pop_back();
            items[3].pop_back();
            int a = stoi(items[1]) -1 ;
            int b = stoi(items[2]) -1;
            int c = stoi(items[3]) -1;
            ModelTriangle t = ModelTriangle(points.at(a), points.at(b), points.at(c), currentColour, objectName);
            triangles.push_back(t);
        }
        if (file.eof() ) break;
    }
    // cout << "triangles, objects and materials loaded" << endl;
    return triangles;
}


CanvasTriangle triangleToCanvas(ModelTriangle t){
  CanvasTriangle projection;
  for(int i = 0; i<3; i++){
    float xWorld = t.vertices[i].x;
    float yWorld = t.vertices[i].y;
    float zWorld = t.vertices[i].z;

    vec3 a(xWorld-cameraPosition.x,yWorld-cameraPosition.y, zWorld- cameraPosition.z);
    a = a * cameraOrientation;
    float ratio = focalLength/a.z;
    int xp = -ratio*a.x + WIDTH/2;
    int yp = ratio*(a.y) +HEIGHT/2;
    CanvasPoint projectedPoint(xp,yp, a.z);
    projection.vertices[i] = projectedPoint;
    projection.colour = t.colour;
  }
  return projection;
}

void drawWireframes(vector<ModelTriangle> triangles){
    vector<float> zArray = getEmptyZArray();
    for (int i = 0; i < triangles.size(); i++){
        ModelTriangle currentTriangle = triangles.at(i);
        CanvasTriangle t = triangleToCanvas(currentTriangle);
        drawWuStrokeTriangle(t);
        //drawFrame(t, zArray);
    }
}

void rasterizeModel(vector<ModelTriangle> triangles){
    vector<float> zArray = getEmptyZArray();
    for (int i = 0; i < triangles.size(); i++){
        ModelTriangle currentTriangle = triangles.at(i);
        CanvasTriangle t = triangleToCanvas(currentTriangle);
        //drawFrame(t, zArray);
        drawFilledTriangle(t, zArray);

    }
}

bool solutionOnTriangle(vec3 i){
    return (0.0<=i.y && i.y<=1.0 && 0.0<=i.z && i.z<=1.0 && (i.y+i.z<=1.0));
}
// find min and max verices in all axes
// interpolate light positions in 3 axes
void initialiseLights(vector<ModelTriangle>triangles, int numberOfLights){
    vector<ModelTriangle> lightTriangles;
    //change these if necessary
    float minX = 9999;
    float maxX = -9999;
    float minY = 9999;
    float maxY = -9999;
    float minZ = 9999;
    float maxZ = -9999;
    for(int i=0; i<triangles.size(); i++){
        ModelTriangle t = triangles.at(i);
        if(!t.objectName.compare("light")){
            //surely theres a better way to do this??
            // comparing vectors somehow
            for (int j=0; j<3; j++){
                if(t.vertices[j].x < minX){
                    minX = t.vertices[j].x;
                }
                if(t.vertices[j].y < minY){
                    minY = t.vertices[j].y;
                }
                if(t.vertices[j].z < minZ){
                    minZ = t.vertices[j].z;
                }
                //max
                if(t.vertices[j].x > maxX){
                    maxX = t.vertices[j].x;
                }
                if(t.vertices[j].y > maxY){
                    maxY = t.vertices[j].y;
                }
                if(t.vertices[j].z > maxZ){
                    maxZ= t.vertices[j].z;
                }
            }
        }
    }
    //vector<vec3> lightDiagonals;
    vector<vec3> bottomLights;
    vector<vec3> rightLights;
    vector<vec3> leftLights;
    vector<vec3> topLights;
    for(int x=0; x<numberOfLights; x++){
        vec3 p;
        p.x = minX +((maxX-minX)*x/(numberOfLights-1));
        p.y = minY +((maxY-minY)*x/(numberOfLights-1));
        p.z = minZ +((maxZ-minZ)*x/(numberOfLights-1));
        bottomLights.push_back(vec3(p.x, p.y, minZ));
        topLights.push_back(vec3(p.x, p.y, maxZ));
        leftLights.push_back(vec3(minX, p.y, p.z));
        rightLights.push_back(vec3(maxX, p.y, p.z));
    }
    for(int j=0; j<leftLights.size(); j++){
        vec3 left = leftLights.at(j);
        vec3 right = rightLights.at(j);
        for(int k=0; k<numberOfLights;k++){
            vec3 light;
            light.x = left.x +((right.x-left.x)*k/(numberOfLights-1));
            light.y = left.y +((right.y-left.y)*k/(numberOfLights-1));
            light.z = left.z +((right.z-left.z)*k/(numberOfLights-1));
            //cout << "(" << light.x << ", " << light.y << ", " << light.z << ")" << endl;
            lightPositions.push_back(light);
        }
    }
    cout << "number of lights: " << lightPositions.size() << endl;
}


float getBrightness(vec3 normal, vec3 lightToIntersection, vec3 ray){
    float brightness = 1/(4 * PI * length(lightToIntersection)*length(lightToIntersection));
    // if (lightingMode == 1){

    float angleOI = dot(normalize(lightToIntersection),normalize(normal));
    if (angleOI > 0){
        // the bigger the angle the closer to parallel the light ray
        // and the normal are so the brighter the the point
        brightness += angleOI;
    }
    // }
    //specular light calcs from http://paulbourke.net/geometry/reflected/
    // else if (lightingMode == 2){
        //POV needs to point towards us
    vec3 flippedRay = -1.0f* ray;
    //do i need to normalize the normal?
    vec3 reflected = lightToIntersection - (2.0f *normalize(normal) * dot(lightToIntersection,normalize(normal)));
    float angleRV = dot( normalize(flippedRay), normalize(reflected));
    if (angleRV > 0.0f){
        brightness += pow(angleRV, 24.0f);
    }
    // }
    //ambient light threshold
    if(brightness < 0.15f){
        brightness = 0.15f;
    }
    return brightness;
}

bool inHardShadow(vector<ModelTriangle> triangles, vec3 surfacePoint, int currentTriangleIndex, vec3 light){
    vec3 shadowRay = light-surfacePoint;
    bool inShadow = false;
    float distanceFromLight = glm::length(shadowRay);
#pragma omp parallel
#pragma omp for
    for(int i=0; i<triangles.size(); i++){
        ModelTriangle triangle = triangles.at(i);
        vec3 e0 = vec3(triangle.vertices[1] - triangle.vertices[0]);
        vec3 e1 = vec3(triangle.vertices[2] - triangle.vertices[0]);
        vec3 SPVector = vec3(surfacePoint-triangle.vertices[0]);
        mat3 DEMatrix(-glm::normalize(shadowRay), e0, e1);
        vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
        float t = possibleSolution.x;
        if(solutionOnTriangle(possibleSolution) && (i != currentTriangleIndex) && t>0.001f){
            if(t < (distanceFromLight)){
                inShadow = true;
            }
        }
    }
    return inShadow;
}

float getSoftShadow(vec3 surfaceNormal, vec3 surfacePoint, vector<ModelTriangle> triangles, int currentTriangleIndex){
    float shadeValue = 0.0f;
    float shadowCount = 0.0f;
    for(int i=0; i<lightPositions.size(); i++){
        if(inHardShadow(triangles, surfacePoint,currentTriangleIndex,lightPositions.at(i))){
            shadowCount ++;
        }
    }
    shadeValue = shadowCount/lightPositions.size();
    return shadeValue;
}

 RayTriangleIntersection getClosestIntersection(vector<ModelTriangle> triangles, vec3 ray){
    RayTriangleIntersection closestIntersection;
    closestIntersection.distanceFromCamera = std::numeric_limits<float>::infinity();
#pragma omp parallel
#pragma omp for
    for (int i=0; i<triangles.size(); i++){
        ModelTriangle triangle = triangles.at(i);
        vec3 e0 = vec3(triangle.vertices[1] - triangle.vertices[0]);
        vec3 e1 = vec3(triangle.vertices[2] - triangle.vertices[0]);
        vec3 SPVector = vec3(cameraPosition-triangle.vertices[0]);
        mat3 DEMatrix(-ray, e0, e1);
        vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
        if (solutionOnTriangle(possibleSolution)){
            if (possibleSolution.x < closestIntersection.distanceFromCamera){

                vec3 intersection = triangle.vertices[0] + possibleSolution.y*e0 + possibleSolution.z*e1;
                vec3 lightToIntersection = lightPosition - intersection;
                //closestIntersection.distanceFromLight = glm::length(lightToIntersection);
                vec3 normal = glm::cross(e0,e1);
                float brightness = getBrightness(normal,lightToIntersection, ray);
                if(shadowMode == 0){
                    bool inShadow = inHardShadow(triangles, intersection, i, lightPosition);
                    if(inShadow){
                        brightness = 0.15f;
                    }
                }
                else if(shadowMode == 1){
                    float shadeValue = getSoftShadow(normal, intersection, triangles, i);
                    float shadeProp = 1 - shadeValue;
                    brightness *= shadeProp;
                    if(brightness < 0.15){
                        brightness = 0.15f;
                    }

                }
                if(brightness > 1.0f){
                    brightness = 1.0f;
                }

                Colour adjustedColour = Colour(triangle.colour.red, triangle.colour.green, triangle.colour.blue, brightness);
                closestIntersection = RayTriangleIntersection(intersection, possibleSolution.x, triangle, adjustedColour);
            } else {
                //interscts & on triangle but not closest
            }
        }
        else {
            // not on triangle
        }
    }
    return closestIntersection;
}

void raytraceModel(vector<ModelTriangle> triangles){
#pragma omp parallel
#pragma omp for
    for(int y= 0; y< HEIGHT; y++){
        for (int x = 0; x < WIDTH; x++){
            vec3 point = vec3(WIDTH/2 -x, y-(HEIGHT/2), focalLength);
            vec3 ray = cameraPosition - point;
            ray = normalize(ray * glm::inverse(cameraOrientation));
            RayTriangleIntersection intersection = getClosestIntersection(triangles, ray );
            if (!std::isinf( intersection.distanceFromCamera )){
                ModelTriangle triangle = intersection.intersectedTriangle;
                window.setPixelColour(x,y, intersection.intersectionPointColour.getPacked());
            }

        }
    }
}
//currently supersampling -- basic
// complex - FXAA http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html
// or quasi monte carlo sampling
void raytraceModelAA(vector<ModelTriangle> triangles){
//Quincux sampling
#pragma omp parallel
#pragma omp for
    for(int y = 0; y < HEIGHT; y++){
        for (int x = 0; x < WIDTH; x++ ){
            vec3 point = vec3(WIDTH/2 -x, y-(HEIGHT/2), focalLength);
            vec3 ray = cameraPosition - point;
            // try changing both x and y -- not really quincux otherwise
            vec3 rayTopLeft = vec3(ray.x-0.5, ray.y, ray.z);
            vec3 rayTopRight = vec3(ray.x+0.5, ray.y, ray.z);
            vec3 rayBottomLeft = vec3(ray.x, ray.y-0.5, ray.z);
            vec3 rayBottomRight = vec3(ray.x, ray.y+0.5, ray.z);

            ray = normalize(ray * glm::inverse(cameraOrientation));
            rayTopLeft = normalize(rayTopLeft* glm::inverse(cameraOrientation));
            rayTopRight = normalize(rayTopRight* glm::inverse(cameraOrientation));
            rayBottomLeft = normalize(rayBottomLeft* glm::inverse(cameraOrientation));
            rayBottomRight = normalize(rayBottomRight* glm::inverse(cameraOrientation));

            RayTriangleIntersection intersection = getClosestIntersection(triangles, ray);
            RayTriangleIntersection intersectionTL = getClosestIntersection(triangles, rayTopLeft);
            RayTriangleIntersection intersectionTR = getClosestIntersection(triangles, rayTopRight);
            RayTriangleIntersection intersectionBL = getClosestIntersection(triangles, rayBottomLeft);
            RayTriangleIntersection intersectionBR = getClosestIntersection(triangles, rayBottomRight);

            if (!std::isinf( intersection.distanceFromCamera )){
                Colour average;
                // find a nicer way to do this
                average.red = (intersection.intersectionPointColour.red*1.4 + intersectionTL.intersectionPointColour.red*0.9 + intersectionTR.intersectionPointColour.red*0.9 + intersectionBL.intersectionPointColour.red*0.9 + intersectionBR.intersectionPointColour.red*0.9)/5.0;
                average.green = (intersection.intersectionPointColour.green*1.4 + intersectionTL.intersectionPointColour.green*0.9 + intersectionTR.intersectionPointColour.green*0.9 + intersectionBL.intersectionPointColour.green*0.9 + intersectionBR.intersectionPointColour.green*0.9)/5.0;
                average.blue = (intersection.intersectionPointColour.blue*1.4 + intersectionTL.intersectionPointColour.blue*0.9 + intersectionTR.intersectionPointColour.blue*0.9 + intersectionBL.intersectionPointColour.blue*0.9 + intersectionBR.intersectionPointColour.blue*0.9)/5.0;
                window.setPixelColour(x,y, average.getPacked());
            }
        }
    }
}

vector<float> getEmptyZArray(){
    vector<float> a;
    for (int i = 0; i < WIDTH*HEIGHT; i++){
        a.push_back(-std::numeric_limits<float>::infinity());
    }
    return a;
}

void rotateInX(float a){

    mat3 m = mat3(vec3(1,      0,       0),
                  vec3(0, cos(a), -sin(a)),
                  vec3(0, sin(a), cos(a)));
    cameraOrientation = cameraOrientation * m ;
}

void rotateInY(float a){
    mat3 m = mat3(vec3(cos(a),  0, sin(a)),
                  vec3(0,       1,      0),
                  vec3(-sin(a), 0, cos(a)));
    cameraOrientation = cameraOrientation * m ;
}

void rotateInZ(float a){
    mat3 m = mat3(  vec3(cos(a), -sin(a), 0),
                    vec3(sin(a), cos(a), 0),
                    vec3(0, 0, 1));
    cameraOrientation = cameraOrientation * m;
}

void lookAt(vec3 p){
  vec3 forward = normalize(cameraPosition - p);
  vec3 right = normalize(cross(vec3(0,1,0), forward));
  vec3 up = normalize(cross(forward, right));
  cameraOrientation[0] = right;
  cameraOrientation[1] = up;
  cameraOrientation[2] = forward;
  printMat3(cameraOrientation);
}

void resetCamera(){
    cameraOrientation = mat3(       vec3(1,0,0),
                                    vec3(0,1,0),
                                    vec3(0,0,1));
    cameraPosition = vec3(0,1,6);
}


void printMat3(mat3 m){
    for (int y =0; y<3;y++){
        cout << "(" << m[y][0] << "," << m[y][1] << "," << m[y][2] << ")" << endl;
    }
    cout << endl;
}

void printMat4(mat4 m){
    for (int y =0; y<4;y++){
        cout << "(" << m[y][0] << "," << m[y][1] << "," << m[y][2] << "," << m[y][3] << ")" << endl;
    }
    cout << endl;
}

void printVec3(vec3 v){
  cout << "(" << v[0] << "," << v[1] << "," << v[2] << ")" << endl;
}
void printVec4(vec4 v){
  cout << "(" << v[0] << "," << v[1] << "," << v[2] << ","<< v[3] << ")" << endl;
}





void handleEvent(SDL_Event event)
{
    if(event.type == SDL_KEYDOWN) {
        if(event.key.keysym.sym == SDLK_LEFT) {
            //cameraPos.x += 1;
            cameraPosition.x -=0.5;
            //lightPosition.x -=0.5;
            cout << "left" << endl;
        }
        else if(event.key.keysym.sym == SDLK_RIGHT) {
            cameraPosition.x += 0.5;

        }
        else if(event.key.keysym.sym == SDLK_UP) {
            cameraPosition.y += 0.25;

        }
        else if(event.key.keysym.sym == SDLK_DOWN) {
            cameraPosition.y -= 0.25;

        }
        else if(event.key.keysym.sym == SDLK_RIGHTBRACKET) {
            cameraPosition.z += 0.5;
        }
        else if(event.key.keysym.sym == SDLK_LEFTBRACKET) {
            cameraPosition.z -= 0.5;
        }
        else if(event.key.keysym.sym == SDLK_c) {
            cout << "C" << endl;
            window.clearPixels();
        }
        else if(event.key.keysym.sym == SDLK_w) {
            cout << "W" << endl;
            float a = 5*PI/180;
            rotateInX(a);
            //printMat3(cameraOrientation);
        }
        else if(event.key.keysym.sym == SDLK_s) {
            cout << "S" << endl;
            float a = -5*PI/180;
            rotateInX(a);
            //printMat3(cameraOrientation);
        }
        else if(event.key.keysym.sym == SDLK_a) {
            cout << "A" << endl;
            float a = 5*PI/180;
            rotateInY(a);
        }
        else if(event.key.keysym.sym == SDLK_d) {
            cout << "D" << endl;
            float a = -5*PI/180;
            rotateInY(a);
            //printMat3(cameraOrientation);
        }
        else if(event.key.keysym.sym == SDLK_q) {
            cout << "Q" << endl;
            float a = 5*PI/180;
            rotateInZ(a);
        }
        else if(event.key.keysym.sym == SDLK_e) {
            cout << "E" << endl;
            float a = -5*PI/180;
            rotateInZ(a);
            //printMat3(cameraOrientation);
        }
        else if(event.key.keysym.sym == SDLK_l) {
            cout << "L" << endl;
            window.clearPixels();
            lookAt(vec3(0, 0, 0));
        }
        // else if(event.key.keysym.sym == SDLK_p) {
        //     cout << "P" << endl;
        //     window.clearPixels();
        //     rasterizeModel(loadObj("cornell-box/cornell-box.obj"));
        // }
        else if(event.key.keysym.sym == SDLK_r) {
            cout << "Reset" << endl;
            resetCamera();
        }
        else if(event.key.keysym.sym == SDLK_m) {
            mode = (mode + 1) %4;
            cout << "Mode:" << mode << endl;
        }
        else if(event.key.keysym.sym == SDLK_1) {
            mode = 1;
            cout << "Wireframe:" << endl;
        }
        else if(event.key.keysym.sym == SDLK_2) {
            mode = 2;
            cout << "Rasterize:" << endl;
        }
        else if(event.key.keysym.sym == SDLK_3) {
            mode = 3;
            cout << "Raytrace:" << endl;
        }
        else if(event.key.keysym.sym == SDLK_4) {
            mode = 4;
            cout << "Raytrace + AA:" << endl;
        }
        else if(event.key.keysym.sym == SDLK_p) {
            savePPM();
        }
    }
    else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
