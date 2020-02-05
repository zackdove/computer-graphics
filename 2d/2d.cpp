#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
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

  CanvasPoint from = CanvasPoint(300, 0);
  CanvasPoint to = CanvasPoint(300, 500);
  float red = 255;
  float green = 255;
  float blue = 255;
  uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
  drawLine(from, to, colour);
  CanvasTriangle t = CanvasTriangle(CanvasPoint(300, 10), CanvasPoint(600, 10), CanvasPoint(450, 310));
  drawStrokeTriangle(t, colour);
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
    cout << a << endl;
    CanvasPoint b = CanvasPoint(rand()%WIDTH, rand()%HEIGHT);
    cout << b << endl;
    CanvasPoint c = CanvasPoint(rand()%WIDTH, rand()%HEIGHT);
    cout << c << endl;
    CanvasTriangle t = CanvasTriangle(a, b, c);
    if (filled){
        drawStrokeTriangle(t, colour);
        drawFilledTriangle(t, colour);
    } else {
        drawStrokeTriangle(t, colour);
    }
}



void drawFilledTriangle(CanvasTriangle t, uint32_t colour){
    CanvasPoint top;
    CanvasPoint middle;
    CanvasPoint bottom;
    CanvasPoint ordered[3];
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
    cout << "top " << top << endl;
    cout << "middle " << middle << endl;
    cout << "bottom " << bottom << endl;
    CanvasPoint d;
    d.y = middle.y;
    float acGradient = (bottom.y-top.y)/(bottom.x-top.x);
    float acIntersection = top.y-top.x*(bottom.y - top.y)/(bottom.x-top.x);
    d.x = (d.y - acIntersection) /acGradient;
    cout << "d.y " << d.y << endl;
    cout << "d.x " << d.x << endl;
    float abGradient = (middle.y-top.y)/(middle.x-top.x);
    float abIntersection = top.y-top.x*(middle.y - top.y)/(middle.x-top.x);
    for (float y=top.y; y <d.y; y++){
        float startX = (y-acIntersection) * (1/acGradient);
        float endX = (y - abIntersection) * (1/abGradient);
        if (startX > endX) {
            swap(startX, endX);
        }
        cout << "startx "<< startX << endl;
        cout << "endx "<< endX << endl;
        for (float x = startX; x < endX; x++){
            window.setPixelColour(round(x), round(y), colour);
        }
    }
    float bcGradient = (bottom.y-middle.y)/(bottom.x-middle.x);
    float bcIntersection = middle.y-middle.x*(bottom.y - middle.y)/(bottom.x-middle.x);
    for (float y=d.y; y < bottom.y; y++){
        float startX = (y-acIntersection) * (1/acGradient);
        float endX = (y - bcIntersection) * (1/bcGradient);
        if (startX > endX) {
            swap(startX, endX);
        }
        for (float x = startX; x < endX; x++){
            window.setPixelColour(round(x), round(y), colour);
        }
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
        window.clearPixels();
        drawRandomTriangle(true);
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
