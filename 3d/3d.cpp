#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include "Image.h"
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include "threeDPoint.h"

using namespace std;
using namespace glm;

#define WIDTH 720
#define HEIGHT 600

void draw();
void update();
void handleEvent(SDL_Event event);
void drawLine(CanvasPoint from, CanvasPoint to, uint32_t colour);
void drawStrokeTriangle(CanvasTriangle t, uint32_t colour);
void drawRandomTriangle(bool filled);
void drawFilledTriangle(CanvasTriangle t, uint32_t colour);
Image loadImage();
void createTextureTriangle();

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[])
{

    SDL_Event event;
    while(true)
    {
        // We MUST poll for events - otherwise the window will freeze !
        if(window.pollForInputEvents(&event)) handleEvent(event);
        update();
        draw();
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}

void draw()
{


}

void update()
{
    // Function for performing animation (shifting artifacts or moving the camera)
}


void drawLine(CanvasPoint from, CanvasPoint to, uint32_t colour){
    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
    float xStepSize = xDiff/numberOfSteps;
    float yStepSize = yDiff/numberOfSteps;
    for (float i=0.0; i<numberOfSteps; i++) {
        float x = from.x + (xStepSize*i);
        float y = from.y + (yStepSize*i);
        window.setPixelColour(round(x), round(y), colour);
    }
}

void drawStrokeTriangle(CanvasTriangle t, uint32_t colour){
    drawLine(t.vertices[0], t.vertices[1], colour);
    drawLine(t.vertices[0], t.vertices[2], colour);
    drawLine(t.vertices[1], t.vertices[2], colour);
}

void drawRandomTriangle(bool filled){

    float red = rand()%255;
    float green = rand()%255;
    float blue = rand()%255;
    uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
    CanvasPoint a = CanvasPoint(rand()%WIDTH, rand()%HEIGHT);
    CanvasPoint b = CanvasPoint(rand()%WIDTH, rand()%HEIGHT);
    CanvasPoint c = CanvasPoint(rand()%WIDTH, rand()%HEIGHT);
    CanvasTriangle t = CanvasTriangle(a, b, c);
    if (filled){
        drawStrokeTriangle(t, colour);
        drawFilledTriangle(t, colour);
    } else {
        drawStrokeTriangle(t, colour);
    }
}


void drawFilledTriangle(CanvasTriangle t, uint32_t colour){
    t.calculateTriangleMeta();
    for (float y=t.top.y; y <t.middleIntersect.y; y++){
        float startX = (y-t.topBottomIntersection) * (1/t.topBottomGradient);
        float endX = (y - t.topMiddleIntersection) * (1/t.topMiddleGradient);
        if (startX > endX) {
            swap(startX, endX);
        }
        for (float x = startX; x < endX; x++){
            window.setPixelColour(round(x), round(y), colour);
        }
    }
    for (float y=t.middleIntersect.y; y < t.bottom.y; y++){
        float startX = (y-t.topBottomIntersection) * (1/t.topBottomGradient);
        float endX = (y - t.middleBottomIntersection) * (1/t.middleBottomGradient);
        if (startX > endX) {
            swap(startX, endX);
        }
        for (float x = startX; x < endX; x++){
            window.setPixelColour(round(x), round(y), colour);
        }
    }
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
    vector<uint32_t> pixels;
    int numOfPixels = width * height;
    int red;
    int green;
    int blue;
    for (size_t i = 0; i < numOfPixels; i++ ){
        red = file.get();
        green = file.get();
        blue = file.get();
        uint32_t pixel = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
        pixels.push_back(pixel);
    }
    Image i = Image(width, height, pixels);
    return i;
}


void displayImage(){
    Image image = loadImage();
    for (int y=0; y<image.height; y++){
        for (int x=0; x<image.width; x++){
            window.setPixelColour(x, y, image.pixels.at((y*image.width)+x));
        }
    }
}

void drawTrianglesForTexture(CanvasTriangle t){
    //Draw fill triangle
    CanvasPoint topP = CanvasPoint(t.top.texturePoint);
    CanvasPoint middleP = CanvasPoint(t.middle.texturePoint);
    CanvasPoint bottomP = CanvasPoint(t.bottom.texturePoint);
    CanvasTriangle mainTriangle = CanvasTriangle(topP, middleP, bottomP);
    uint32_t colour = (255<<24) + (int(255)<<16) + (int(255)<<8) + int(255);
    drawFilledTriangle(mainTriangle, colour);
    //Draw two stroke triangles for halves
    CanvasPoint middleIntersectP = CanvasPoint(t.middleIntersect.texturePoint);
    //Top
    colour = (255<<24) + (int(0)<<16) + (int(0)<<8) + int(255);
    CanvasTriangle topTriangle = CanvasTriangle(topP, middleP, middleIntersectP);
    drawStrokeTriangle(topTriangle, colour);
    //Bottom
    colour = (255<<24) + (int(0)<<16) + (int(255)<<8) + int(0);
    CanvasTriangle bottomTriangle = CanvasTriangle(bottomP, middleP, middleIntersectP);
    drawStrokeTriangle(bottomTriangle, colour);

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
    drawTrianglesForTexture(t);
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
            window.setPixelColour(starts.at(y).x+x, starts.at(y).y, i.getPixel(row.at(x).x, row.at(x).y));
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
            window.setPixelColour(bottomStarts.at(y).x+x, bottomStarts.at(y).y-1, i.getPixel(row.at(x).x, row.at(x).y));
        }
    }
}

vector<Colour> loadMtl(string path){
    // name = "cornell_box/cornell_box.mtl"
    string nameLine;
    string propertiesLine;
    ifstream file;
    file.open(path);
    if (file.fail()){
        cout << ".MTL FILE COULD NOT BE OPENED" << endl;
    }
    vector<Colour> colours;
    while (true) {
        getline(file, nameLine);
        //Char 32 is space
        char space = char(32);
        string name = split(nameLine, space)[1];
        cout << name << endl;
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

vector<ModelTriangle> loadObj(){
    ifstream file;
    file.open("cornell-box/cornell-box.obj");
    char space = char(32);
    vector<string> materialFileNames;
    vector<vec3> points;
    vector<Colour> colours;
    vector<ModelTriangle> triangles;
    //Get vertices, and material files
    while (true){
        string line;
        getline(file, line);
        string* items = split(line, space);
        if (!items[0].compare("mtllib")){
            vector<Colour> newColours = loadMtl(items[1]);
            colours.insert(colours.end(), newColours.begin(), newColours.end() );
        } else if (!items[0].compare("v")){
            vec3 point = vec3( stof(items[1]), stof(items[2]), stof(items[3]) );
            // vec3 point = vec3(1, 1, 1);
            points.push_back(point);
        }
        if (file.eof() ) break;
    }
    cout << "Vertices and material files loaded" << endl;
    //Get triangles, and their objects & materials, reset file pointer first
    file.clear();
    file.seekg(0, ios::beg);
    while (true){
        string line;
        getline(file, line);
        string* items = split(line, space);
        string objectName;
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
            items[1].pop_back();
            items[2].pop_back();
            items[3].pop_back();
            int a = stoi(items[1]) -1 ;
            int b = stoi(items[2]) -1;
            int c = stoi(items[3]) -1;
            ModelTriangle t = ModelTriangle(points.at(a), points.at(b), points.at(c), currentColour, objectName);
            triangles.push_back(t);
            cout << t << endl;
        }
        if (file.eof() ) break;
    }
    cout << "triangles, objects and materials loaded" << endl;
    return triangles;
}


CanvasTriangle triangleToCanvas(ModelTriangle t){
  //chnage this to something more meaningful
  vec3 camera(0,0, 10);
  float focalLength = 400;
  // change this to a for loop
  CanvasTriangle currentTriangle;
  for (int i = 0; i < 3; i++){
    float xWorld = t.vertices[i].x;
    float yWorld = t.vertices[i].y;
    float zWorld = t.vertices[i].z;
    float xDistanceFromCamera = xWorld - camera.x;
    float yDistanceFromCamera = yWorld -camera.y;
    float zDistanceFromCamera = zWorld - camera.z;
    float ratio = focalLength/ -zDistanceFromCamera;
    // change to int?
    int xImage = (xDistanceFromCamera*ratio) + WIDTH/2;
    int yImage = ((1-yDistanceFromCamera)*ratio) + HEIGHT/2;
    CanvasPoint currentPoint(xImage, yImage);
    currentTriangle.vertices[i] = currentPoint;
  }
  return currentTriangle;
}

void drawWireframes(){
    vector<ModelTriangle> triangles = loadObj();
    for (int i = 0; i < triangles.size(); i++){
        ModelTriangle currentTriangle = triangles.at(i);
        CanvasTriangle t = triangleToCanvas(currentTriangle);
        uint32_t colour = (255<<24) + (int(currentTriangle.colour.red)<<16) + (int(currentTriangle.colour.green)<<8) + int(currentTriangle.colour.blue);
        drawStrokeTriangle(t, colour);
    }
}


void handleEvent(SDL_Event event)
{
    if(event.type == SDL_KEYDOWN) {
        if(event.key.keysym.sym == SDLK_LEFT) cout << "LEFT" << endl;
        else if(event.key.keysym.sym == SDLK_RIGHT) cout << "RIGHT" << endl;
        else if(event.key.keysym.sym == SDLK_UP) cout << "UP" << endl;
        else if(event.key.keysym.sym == SDLK_DOWN) cout << "DOWN" << endl;
        else if(event.key.keysym.sym == SDLK_u) {
            cout << "U" << endl;
            drawRandomTriangle(false);
        }
        else if(event.key.keysym.sym == SDLK_f) {
            cout << "F" << endl;
            drawRandomTriangle(true);
        }
        else if(event.key.keysym.sym == SDLK_i) {
            cout << "I" << endl;
            displayImage();
        }
        else if(event.key.keysym.sym == SDLK_t) {
            cout << "T" << endl;
            createTextureTriangle();
        }
        else if(event.key.keysym.sym == SDLK_c) {
            cout << "C" << endl;
            window.clearPixels();
        }
        else if(event.key.keysym.sym == SDLK_m) {
            cout << "M" << endl;
            loadMtl("cornell-box/cornell-box.mtl");
        }
        else if(event.key.keysym.sym == SDLK_o) {
            cout << "O" << endl;
            loadObj();
        }
        else if(event.key.keysym.sym == SDLK_w) {
            cout << "W" << endl;
            drawWireframes();
        }
    }
    else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
