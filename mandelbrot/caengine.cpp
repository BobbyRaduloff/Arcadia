#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;

int MAX_ITER = 80;

#define CURRENT (i * SCREEN_WIDTH + j)

double flMap(double x, double a1, double a2, double b1, double b2) {
    return b1 + ((x - a1) * (b2 - b1))/(a2 - a1);
}

class RGB {
    public:
    uint8_t r, g, b;
};


RGB getColor(int n) {
    double hue = flMap(n, 0, MAX_ITER, 0, 360);
    RGB ret;
    if(hue < 60.f) {
        ret.r = 255;
        ret.g = flMap(hue, 0, 60, 0, 255);
        ret.b = 0;
    } else if(hue < 120.f) {
        ret.r = flMap(hue, 60, 120, 255, 0);
        ret.g = 255;
        ret.b = 0;
    } else if(hue < 180.f) {
        ret.r = 0;
        ret.g = 255;
        ret.b = flMap(hue, 120, 180, 0, 255);
    } else if(hue < 240.f) {
        ret.r = 0;
        ret.g = flMap(hue, 180, 240, 255, 0);
        ret.b = 255;
    } else if(hue < 300.f) {
        ret.r = flMap(hue, 240, 300, 0, 255);
        ret.g = 0;
        ret.b = 255;
    } else {
        ret.r = 255;
        ret.g = 0;
        ret.b = flMap(hue, 300, 360, 255, 0);
    }

    return ret;
}

int main()
{
    sf::RenderWindow window(
        sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT),
        "CA Engine",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    
    uint8_t screenData[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
    sf::Texture screenTexture;
    screenTexture.create(SCREEN_WIDTH, SCREEN_HEIGHT);
    sf::Sprite screenSprite(screenTexture);

    double minx = -2.f;
    double maxx = 2.f;
    double miny = -2.f;
    double maxy = 2.f;
    double prevMinx = minx, prevMaxx = maxx, prevMiny = miny, prevMaxy = maxy;
    double zoomFactor;
    int zoomLevel = 1;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch(event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }
                case sf::Event::MouseButtonPressed: {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    prevMinx = minx, prevMaxx = maxx, prevMiny = miny, prevMaxy = maxy;
                    double nx = (double) mousePos.x;
                    double ny = (double) mousePos.y;
                    double nxprime = flMap(nx, 0, SCREEN_WIDTH, minx, maxx);
                    double nyprime = flMap(ny, 0, SCREEN_HEIGHT, miny, maxy);
                    double width = maxx - minx;
                    double height = maxy - miny;
                    zoomFactor = flMap(zoomLevel, 1, 100, 0.5f, 0.01f);
                    MAX_ITER = flMap(zoomLevel, 1, 100, 80, 3000);
                    minx = nxprime - (zoomFactor * width) / 2;
                    maxx = nxprime + (zoomFactor * width) / 2;
                    miny = nyprime - (zoomFactor * height) / 2;
                    maxy = nyprime + (zoomFactor * height) / 2;
                    std::cout << "miny(" << miny << ") = nyprime(" << nyprime << ") - (zoomFactor(" << zoomFactor << ") * height(" << height << ")) / 2" << std::endl;
                    std::cout << "minx(" << minx << ") = nxprime(" << nxprime << ") - (zoomFactor(" << zoomFactor << ") * width(" << width << ")) / 2" << std::endl;
                    std::cout << "maxy(" << maxy << ") = nyprime(" << nyprime << ") + (zoomFactor(" << zoomFactor << ") * height(" << height << ")) / 2" << std::endl;
                    std::cout << "maxx(" << maxx << ") = nxprime(" << nxprime << ") + (zoomFactor(" << zoomFactor << ") * width(" << width << ")) / 2" << std::endl;
                    std::cout << "width: " << width << " height: " << height << std::endl;
                    std::cout << "zoomLevel: " << zoomLevel << " zoomFactor: " << zoomFactor << " MAX_ITER: " << MAX_ITER << std::endl;
                    std::cout << "-------------" << std::endl;
                    zoomLevel++;
                    break;
                }
                case sf::Event::KeyPressed: {
                    minx = prevMinx, maxx = prevMaxx, miny = prevMiny, maxy = prevMaxy;
                    zoomLevel--;
                    break;
                }
                default: break;
            }

        }
        
        //std::cout << "x e (" << minx << ", " << maxx << ")" << " y e (" << miny << ", " << maxy << ")" << std::endl; 

        for(int i = 0; i < SCREEN_HEIGHT; i++) {
            for(int j = 0; j < SCREEN_WIDTH; j++) {
                // i = c.imaginary
                // j = c.real
                double a = flMap(j, 0, SCREEN_WIDTH, minx, maxx);
                double b = flMap(i, 0, SCREEN_HEIGHT, miny, maxy);
                double ca = a;
                double cb = b;
                int n;
                for(n = 0; n < MAX_ITER; n++) {
                    double aa = a * a - b * b;
                    double bb = 2 * a * b;
                    a = aa + ca;
                    b = bb + cb;
                    if(abs(a + b) > 2) {
                        break;
                    }
                }
                // draw Cells
                RGB color;
                if(n < MAX_ITER)
                    color = getColor(n);
                else
                    color = {0, 0, 0};
                screenData[CURRENT * 4 + 0] = color.r;
                screenData[CURRENT * 4 + 1] = color.g;
                screenData[CURRENT * 4 + 2] = color.b;
                screenData[CURRENT * 4 + 3] = 255;
            }
        }

        screenTexture.update(screenData);

        window.clear();
        
        window.draw(screenSprite);
        
        window.display();
    }

    return 0;
}