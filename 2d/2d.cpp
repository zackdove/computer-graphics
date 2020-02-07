#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include "Image.h"
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <string>

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

void createTextureTriangle(){
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
    float topBottomDist()
    float middleIntersectRatioX = (t.middleIntersect.x-t.top.x)/(t.bottom.x-t.top.x);
    float middleIntersectRatioY = (t.middleIntersect.y-t.top.y)/(t.bottom.y-t.top.y);
    float topBottomDistXTexture = (t.bottom.texturePoint.x - t.top.texturePoint.x);
    float topBottomDistYTexture = (t.bottom.texturePoint.y - t.top.texturePoint.y);
    t.middleIntersect.texturePoint.x = middleIntersectRatioX * topBottomDistXTexture;
    t.middleIntersect.texturePoint.y = middleIntersectRatioY * topBottomDistYTexture;
    drawTrianglesForTexture(t);
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
    }
    else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
