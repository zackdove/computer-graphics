#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include "Image.h"
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

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

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[])
{
    loadImage();
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


//Calculates the points: top, middle, middleintersect, bottom, and the gradients and intersections of all sides
CanvasTriangle calculateTriangleMeta(CanvasTriangle t){
    CanvasPoint top;
    CanvasPoint middle;
    CanvasPoint bottom;
    CanvasPoint a = t.vertices[0];
    CanvasPoint b = t.vertices[1];
    CanvasPoint c = t.vertices[2];
    vector<float> ys = {a.y, b.y, c.y};
    float max = *max_element(ys.begin(), ys.end());
    if (a.y == max){
        bottom =a;
        if (b.y > c.y){
            middle = b;
            top = c;
        } else {
            middle = c;
            top = b;
        }
    } else if (b.y == max){
        bottom = b;
        if (a.y>c.y){
            middle = a;
            top = c;
        } else {
            middle = c;
            top = a;
        }
    } else {
        bottom = c;
        if (a.y>b.y){
            middle = a ;
            top = b;
        }
        else {
            top =a;
            middle = b;
        }
    }
    CanvasPoint middleIntersect;
    middleIntersect.y = middle.y;
    float topBottomGradient = (bottom.y-top.y)/(bottom.x-top.x);
    float topBottomIntersection = top.y-top.x*(bottom.y - top.y)/(bottom.x-top.x);
    middleIntersect.x = (middleIntersect.y - topBottomIntersection) /topBottomGradient;
    float topMiddleGradient = (middle.y-top.y)/(middle.x-top.x);
    float topMiddleIntersection = top.y-top.x*(middle.y - top.y)/(middle.x-top.x);
    float middleBottomGradient = (bottom.y-middle.y)/(bottom.x-middle.x);
    float middleBottomIntersection = middle.y-middle.x*(bottom.y - middle.y)/(bottom.x-middle.x);

    t.top = top;
    t.middle = middle;
    t.middlIntersect = middleIntersect;
    t.bottom = bottom;

    t.topBottomGradient = topBottomGradient;
    float topBottomIntersection;
    float topMiddleGradient;
    float topMiddleIntersection;
    float middleBottomGradient;
    float middleBottomIntersection;


    return sorted;
}

void drawFilledTriangle(CanvasTriangle t, uint32_t colour){
    vector<CanvasPoint> sortedVertices = sortVertices(t);
    CanvasPoint top = sortedVertices.at(0);
    CanvasPoint middle = sortedVertices.at(1);
    CanvasPoint middleIntersect = sortedVertices.at(2);
    CanvasPoint bottom = sortedVertices.at(3);

    float topMiddleGradient =
    float topMiddleIntersection =
    for (float y=top.y; y <middleIntersect.y; y++){
        float startX = (y-topBottomIntersection) * (1/topBottomGradient);
        float endX = (y - topMiddleIntersection) * (1/topMiddleGradient);
        if (startX > endX) {
            swap(startX, endX);
        }

        for (float x = startX; x < endX; x++){
            window.setPixelColour(round(x), round(y), colour);
        }
    }
    float middleBottomGradient =
    float middleBottomIntersection =
    for (float y=middleIntersect.y; y < bottom.y; y++){
        float startX = (y-topBottomIntersection) * (1/topBottomGradient);
        float endX = (y - middleBottomIntersection) * (1/middleBottomGradient);
        if (startX > endX) {
            swap(startX, endX);
        }
        for (float x = startX; x < endX; x++){
            window.setPixelColour(round(x), round(y), colour);
        }
    }
}

Image loadImage(){
    char pSix[3];
    char note[100];
    int width;
    int height;
    int colourRange;
    const char* filename = "texture.ppm";
    FILE* file = fopen(filename, "rb");
    fscanf(file, "%s\n", pSix);
    // fscanf(file, "%s", note);
    fgets(note, 100, file);
    fscanf(file, "%d %d\n %d", &width, &height, &colourRange);
    puts(note);
    cout << "width " << width << endl;
    cout << "height " << height << endl;
    cout << "colourRange " << colourRange << endl;
    int numOfPixels = width*height;

    vector<uint32_t> pixels;
    int red = 0;
    int green = 0;
    int blue = 0;
    for (int i=0; i<numOfPixels; i++){
        fread(&red, 1, 1, file);
        fread(&green, 1, 1, file);
        fread(&blue, 1, 1, file);
        // cout << "red " << red << endl;
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
            window.setPixelColour(round(x), round(y), image.pixels.at((y*image.width)+x));
        }
    }
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
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
