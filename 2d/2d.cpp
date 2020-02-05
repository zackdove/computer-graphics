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
void drawTriangle(CanvasTriangle t, uint32_t colour);
void drawRandomTriangle();

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
  drawTriangle(t, colour);
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

void drawTriangle(CanvasTriangle t, uint32_t colour){
    drawLine(t.vertices[0], t.vertices[1], colour);
    drawLine(t.vertices[0], t.vertices[2], colour);
    drawLine(t.vertices[1], t.vertices[2], colour);
}

void drawRandomTriangle(){
    cout << "kjdhsskjdhj" << endl;
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
    drawTriangle(t, colour);
}



void drawFilledTriangle(CanvasTriangle t, uint32_t colour){
    CanvasPoint top;
    CanvasPoint middle;
    CanvasPoint bottom;
    CanvasPoint ordered[3];
    CanvasPoint a = t.vertices[0];
    CanvasPoint b = t.vertices[1];
    CanvasPoint c = t.vertices[2];
    top = a;
    middle = b;
    bottom = c;

    if (a.y > 

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
        drawRandomTriangle();
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
