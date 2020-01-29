#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace glm;

#define WIDTH 320
#define HEIGHT 240

void draw();
void update();
void handleEvent(SDL_Event event);
vector<float> interpolate(float start, float end, int steps);
void greyScaleX();

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[])
{
    interpolate(2.2, 8.5, 7);
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
    window.clearPixels();
    greyScaleX();
}

void redNoise(){
    for(int y=0; y<window.height ;y++) {
        for(int x=0; x<window.width ;x++) {
            float red = rand() % 255;
            float green = 0.0;
            float blue = 0.0;
            uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
            window.setPixelColour(x, y, colour);
        }
    }
}

void update()
{
    // Function for performing animation (shifting artifacts or moving the camera)
}

void greyScaleX(){
    vector<float> greyValues = interpolate(255, 0, window.width);
    for(int x=0; x<window.width ;x++) {
        float red = greyValues.at(x);
        float green = greyValues.at(x);
        float blue = greyValues.at(x);
        //Can simplify
        uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
        for(int y=0; y<window.height ;y++) {
            window.setPixelColour(x, y, colour);
        }
    }
}

vec3 uintToVec3(uint32_t){
    vec3 v
}

void rainbow(){
    //top left red
    uint32_t red = (255<<24) + (int(255)<<16) + (int(0)<<8) + int(0));
    window.setPixelColour(0,0, red);
    //top right blue
    uint32_t blue = (255<<24) + (int(0)<<16) + (int(0)<<8) + int(255);
    window.setPixelColour(window.width-1,0, blue);
    //bottom left yellow
    uint32_t yellow = (255<<24) + (int(0)<<16) + (int(255)<<8) + int(255);
    window.setPixelColour(window.width-1,0, yellow);
    //bottom right green
    uint32_t green = (255<<24) + (int(0)<<16) + (int(255)<<8) + int(0);
    window.setPixelColour(window.width-1,0, green);
    for(int y=0; y<window.height ;y++) {
        for(int x=0; x<window.width ;x++) {
            vec3 left =
        }
    }
}

vector<float> interpolate(float start, float end, int steps){
    vector<float> v = {};
    for (int i=0; i<steps; i++){
        float num = start+((end-start)*i/(steps-1));
        // cout << num << endl;
        v.push_back(num);
    }
    return v;
}

void handleEvent(SDL_Event event)
{
    if(event.type == SDL_KEYDOWN) {
        if(event.key.keysym.sym == SDLK_LEFT) cout << "LEFT" << endl;
        else if(event.key.keysym.sym == SDLK_RIGHT) cout << "RIGHT" << endl;
        else if(event.key.keysym.sym == SDLK_UP) cout << "UP" << endl;
        else if(event.key.keysym.sym == SDLK_DOWN) cout << "DOWN" << endl;
    }
    else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
