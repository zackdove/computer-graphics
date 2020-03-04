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
void createTextureTriangle();
vector<float> getEmptyZArray();
vector<ModelTriangle> loadObj(string path);
void drawRow(CanvasPoint from, CanvasPoint to, Colour colour, vector<float> &zArray);
void drawModel(vector<ModelTriangle> triangles);
void printMat3(mat3 m);
void printVec3(vec3 v);
void printMat4(mat4 m);
void printVec4(vec4 v);

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
vec3 cameraPos = vec3(0,0,HEIGHT/30);
mat3 cameraOrientation = mat3(  1, 0, 0,
                                0, 1, 0,
                                0, 0, 1);

vec4 cameraPos2 = vec4(0,0,6,1);

mat4 cameraHom = transpose(mat4(  vec4(1,0,0,0),
                                  vec4(0,1,0,0),
                                  vec4(0,0,1,0),
                                  -cameraPos2));




int main(int argc, char* argv[])
{
    vector<ModelTriangle> model = loadObj("cornell-box/cornell-box.obj");

    SDL_Event event;
    while(true)
    {
        // We MUST poll for events - otherwise the window will freeze !
        if(window.pollForInputEvents(&event)) {
            handleEvent(event);
        }
        update();
        draw();
        window.clearPixels();
        drawModel(model);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}

void draw()
{

}

void update()
{
  cameraHom[0][3] = -cameraPos2[0];
  cameraHom[1][3] = -cameraPos2[1];
  cameraHom[2][3] = -cameraPos2[2];
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

void drawStrokeTriangle(CanvasTriangle t){
    drawLine(t.vertices[0], t.vertices[1], t.colour);
    drawLine(t.vertices[0], t.vertices[2], t.colour);
    drawLine(t.vertices[1], t.vertices[2], t.colour);
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
    // float fromDepth = (from.depth);
    // float toDepth = (to.depth);
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
        // cout << "R " << v.x << endl;
        // cout << "G " << v.y << endl;
        // cout << "B " << v.z << endl;
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
            // cout << t << endl;
        }
        if (file.eof() ) break;
    }
    // cout << "triangles, objects and materials loaded" << endl;
    return triangles;
}

CanvasTriangle triangleToCanvas(ModelTriangle t){
    //chnage this to something more meaningful
    float focalLength = HEIGHT/2;
    // change this to a for loop
    CanvasTriangle projection;
    projection.colour = t.colour;
    for (int i = 0; i < 3; i++){
        vec3 cameraToVertex = t.vertices[i] - cameraPos;
        vec3 adjustedVector = cameraToVertex * cameraOrientation;
        float ratio = focalLength/ -cameraToVertex.z;
        // change to int?
        int xImage = (adjustedVector.x*ratio) + WIDTH/2;
        int yImage = ((1-adjustedVector.y)*ratio) + HEIGHT/2;
        CanvasPoint currentPoint(xImage, yImage, adjustedVector.z);
        projection.vertices[i] = currentPoint;
    }
    return projection;
}

CanvasPoint toImageCoords(CanvasPoint p)
{
	int w = WIDTH / 2;
	int h = HEIGHT / 2;
	float xp = w + (p.x);
	float yp = h + (p.y);
	return CanvasPoint(xp, yp, p.depth);
}

CanvasPoint project3DPoint(vec3 p){
  float focalLength = 250;
  vec4 homP = vec4(p.x,p.y,p.z,1);
  vec4 a = homP * cameraHom;
  float ratio = -focalLength/a.z;
  CanvasPoint A = CanvasPoint(a.x*ratio, (1-a.y)*ratio, a.z);
  return A;
}



CanvasTriangle triangleToCanvas2(ModelTriangle t){
  CanvasPoint A = project3DPoint(t.vertices[0]);
  CanvasPoint B = project3DPoint(t.vertices[1]);
  CanvasPoint C = project3DPoint(t.vertices[2]);
  A = toImageCoords(A);
  B = toImageCoords(B);
  C = toImageCoords(C);
  CanvasTriangle projection = CanvasTriangle(A,B,C, t.colour);
  return projection;
}

void drawWireframes(vector<ModelTriangle> triangles){
    for (int i = 0; i < triangles.size(); i++){
        ModelTriangle currentTriangle = triangles.at(i);
        CanvasTriangle t = triangleToCanvas2(currentTriangle);
        drawStrokeTriangle(t);
    }
}

void drawModel(vector<ModelTriangle> triangles){
    vector<float> zArray = getEmptyZArray();
    for (int i = 0; i < triangles.size(); i++){
        ModelTriangle currentTriangle = triangles.at(i);
        CanvasTriangle t = triangleToCanvas2(currentTriangle);
        drawFilledTriangle(t, zArray);
    }
}



vector<float> getEmptyZArray(){
    vector<float> a;
    for (int i = 0; i < WIDTH*HEIGHT; i++){
        a.push_back(-std::numeric_limits<float>::infinity());
    }
    return a;
}

//In radians
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


void rotateInXH(float a){

    mat4 m = mat4(vec4(1,      0,       0, 0),
                  vec4(0, cos(a), -sin(a), 0),
                  vec4(0, sin(a), cos(a),  0),
                  vec4(0, 0,0,1));
    cameraHom = cameraHom * m ;
}

void rotateInYH(float a){
    mat4 m = mat4(vec4(cos(a),  0, sin(a),0),
                  vec4(0,       1,      0,0),
                  vec4(-sin(a), 0, cos(a),0),
                  vec4(0,0,0,1));
    cameraHom = cameraHom * m ;
}

void rotateInZH(float a){
    mat4 m = mat4(  vec4(cos(a), -sin(a), 0,0),
                    vec4(sin(a), cos(a), 0,0),
                    vec4(0, 0, 1,0),
                    vec4(0,0,0,1));
    cameraHom = cameraHom * m;
}

void lookAt(vec3 p){

    vec3 forward = normalize(cameraPos - p);
    vec3 right = normalize(cross(vec3(0, 1, 0), forward));
    vec3 up = normalize(cross(forward, right));
    printMat3(cameraOrientation);
    cameraOrientation = mat3(   right.x, up.x, forward.x,
                                right.y, up.y, forward.y ,
                                right.z, up.z, forward.z);
    printMat3(cameraOrientation);
    printVec3(cameraPos);
}

void lookAt2(vec3 p){
  vec3 camera = vec3(cameraPos2.x, cameraPos2.y, cameraPos2.z);
  vec3 forward = normalize(camera - p);
  vec3 right = normalize(cross(vec3(0,1,0), forward));
  vec3 up = normalize(cross(forward, right));

  // cameraHom[0][0] = right.x;
  // cameraHom[1][0] = right.y;
  // cameraHom[2][0] = right.z;
  // cameraHom[0][1] = up.x;
  // cameraHom[1][1] = up.y;
  // cameraHom[2][1] = up.z;
  // cameraHom[0][2] = forward.x;
  // cameraHom[1][2] = forward.y;
  // cameraHom[2][2] = forward.z;
  cameraHom[0][0] = right.x;
  cameraHom[0][1] = right.y;
  cameraHom[0][2] = right.z;
  cameraHom[1][0] = up.x;
  cameraHom[1][1] = up.y;
  cameraHom[1][2] = up.z;
  cameraHom[2][0] = forward.x;
  cameraHom[2][1] = forward.y;
  cameraHom[2][2] = forward.z;
  //potentially already updated so no need to do this
  // cameraHom[0][3] = camera.x;
  // cameraHom[1][3] = camera.y;
  // cameraHom[2][3] = camera.z;
  printMat4(cameraHom);
}

void printMat3(mat3 m){
    for (int y =0; y<3;y++){
        cout << "(" << m[y][0] << "," << m[y][1] << "," << m[y][2] << ")" << endl;
    }
}

void printMat4(mat4 m){
    for (int y =0; y<4;y++){
        cout << "(" << m[y][0] << "," << m[y][1] << "," << m[y][2] << "," << m[y][3] << ")" << endl;
    }
}

void printVec3(vec3 v){
  cout << "(" << v[0] << "," << v[1] << "," << v[2] << ")" << endl;
}
void printVec4(vec4 v){
  cout << "(" << v[0] << "," << v[1] << "," << v[2] << ","<< v[3] << ")" << endl;
}

void resetCamera(){
    cameraOrientation = mat3( 1, 0, 0,
                              0, 1, 0,
                              0, 0, 1);
    cameraPos = vec3(0,0,10);
}

void resetCamera2(){
  cameraPos2 = vec4(0,0,6,1);

  cameraHom = transpose(mat4(  vec4(1,0,0,0),
                                    vec4(0,1,0,0),
                                    vec4(0,0,1,0),
                                    -cameraPos2));
}

bool solutionOnTriangle(vec3 i){
    return (0<=i.y && i.y<=1 && 0<=i.z && i.z<=1 && (i.y+i.z<=1));
}

void getClosestIntersection(vector<ModelTriangle> triangles, vec3 ray){
    float closestDist = std::numeric_limits<float>::infinity();
    for (int i=0; i<triangles.size(); i++){
        ModelTriangle triangle = triangles.at(i);
        vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
        vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
        //CameraPos2 is in homogenous, everything else needs to be too
        vec3 SPVector = cameraPosition-triangle.vertices[0];
        mat3 DEMatrix(-ray, e0, e1);
        vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
        if (solutionOnTriangle(possibleSolution)){
            if (possibleSolution.x < closestDist){
                closestDist = possibleSolution.x;
                //draw
            } else {
                //interscts & on triangle but not closest
            }
        } else {
            // not on triangle
        }
    }

}



void handleEvent(SDL_Event event)
{
    if(event.type == SDL_KEYDOWN) {
        if(event.key.keysym.sym == SDLK_LEFT) {
            //cameraPos.x += 1;
            cameraPos2.x -= 0.5;
            cout << "left" << endl;

        }
        else if(event.key.keysym.sym == SDLK_RIGHT) {
            //cameraPos.x -= 1;
            cameraPos2.x += 0.5;
        }
        else if(event.key.keysym.sym == SDLK_UP) {
            //cameraPos.y -= 1;
            cameraPos2.y += 0.25;
        }
        else if(event.key.keysym.sym == SDLK_DOWN) {
            //cameraPos.y += 1;
            cameraPos2.y -= 0.25;
        }
        else if(event.key.keysym.sym == SDLK_RIGHTBRACKET) {
            //cameraPos.z += 1;
            cameraPos2.z += 0.5;
        }
        else if(event.key.keysym.sym == SDLK_LEFTBRACKET) {
            //cameraPos.z -= 1;
            cameraPos2.z -= 0.5;
        }
        else if(event.key.keysym.sym == SDLK_c) {
            cout << "C" << endl;
            window.clearPixels();
        }
        else if(event.key.keysym.sym == SDLK_w) {
            cout << "W" << endl;
            float a = 5*PI/180;
            rotateInXH(a);
            //printMat3(cameraOrientation);
            printMat4(cameraHom);
        }
        else if(event.key.keysym.sym == SDLK_s) {
            cout << "S" << endl;
            float a = -5*PI/180;
            rotateInXH(a);
            //printMat3(cameraOrientation);
            printMat4(cameraHom);
        }
        else if(event.key.keysym.sym == SDLK_a) {
            cout << "A" << endl;
            float a = 5*PI/180;
            rotateInYH(a);
            printMat3(cameraOrientation);
        }
        else if(event.key.keysym.sym == SDLK_d) {
            cout << "D" << endl;
            float a = -5*PI/180;
            rotateInYH(a);
            //printMat3(cameraOrientation);
            printMat4(cameraHom);
        }
        else if(event.key.keysym.sym == SDLK_q) {
            cout << "Q" << endl;
            float a = 5*PI/180;
            rotateInZH(a);
            printMat3(cameraOrientation);
        }
        else if(event.key.keysym.sym == SDLK_e) {
            cout << "E" << endl;
            float a = -5*PI/180;
            rotateInZH(a);
            //printMat3(cameraOrientation);
            printMat4(cameraHom);
        }
        else if(event.key.keysym.sym == SDLK_l) {
            cout << "L" << endl;
            window.clearPixels();
            lookAt2(vec3(0, 0, 0));
        }
        else if(event.key.keysym.sym == SDLK_p) {
            cout << "P" << endl;
            window.clearPixels();
            drawModel(loadObj("cornell-box/cornell-box.obj"));
        }
        else if(event.key.keysym.sym == SDLK_r) {
            cout << "Reset" << endl;
            resetCamera2();
        }
    }
    else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
