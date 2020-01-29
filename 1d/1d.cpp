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

vector<float> interpolate(float start, float end, int steps){
    vector<float> v = {};
    for (int i=0; i<steps; i++){
        float num = start+((end-start)*i/(steps-1));
        cout << num << endl;
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
