#include "imgui.h"
#include "imgui-SFML.h"
#include "perlin.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cinttypes>
#include <vector>
#include <numeric>
#include <cmath>
#include <string>

#include "cell.hpp"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const float HEAT_FLOW = 0.12f;
const float DENSITY_CONSTANT = 0.001f;
const float MATH_E = 2.7183f;

class RGB {
    public:
    uint8_t r, g, b;
};

RGB tempToRGBA(int temp, int minTemp = 0, int maxTemp = 100) {
    float parameter = (float)(temp - minTemp) / (float)(maxTemp - minTemp);
    int result = parameter*510;
    RGB ret;
    if(result <= 255) {
        ret.r = result;
        ret.g = ret.b = 0;
    } else {
        ret.r = 255;
        ret.g = ret.b = result - 255;
    }

    return ret;
}

#define BOUND(x) (x >= 0 && x < SCREEN_WIDTH * SCREEN_HEIGHT)
#define CURRENT (i * SCREEN_WIDTH + j)
#define SIMULATE entropy

Cell cells[SCREEN_HEIGHT][SCREEN_WIDTH];
Cell newCells[SCREEN_HEIGHT][SCREEN_WIDTH];

void entropy(int ci, int cj, int ni, int nj) {
    float deltaT = cells[ci][cj].temperature - cells[ni][nj].temperature;
    float nH = cells[ni][nj].specificHeatCapacity * cells[ni][nj].mass;
    if(deltaT > 0.0005f) {
        float dQ = deltaT * nH;
        float Q = dQ * HEAT_FLOW;
        newCells[ci][cj].temperature -= Q / (cells[ci][cj].mass * cells[ci][cj].specificHeatCapacity);
        newCells[ni][nj].temperature += Q / (cells[ni][nj].mass * cells[ni][nj].specificHeatCapacity);
    } else if(cells[ci][cj].temperature - cells[ni][nj].temperature > 0 && cells[ci][cj].temperature - cells[ni][nj].temperature < 0.01f) {
        newCells[ci][cj].temperature = cells[ni][nj].temperature = (cells[ci][cj].temperature + cells[ni][nj].temperature) / 2;
    }
}

int main()
{
    sf::RenderWindow window(
        sf::VideoMode(SCREEN_WIDTH + 470, SCREEN_HEIGHT),
        "CA Engine",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);
    
    uint8_t screenData[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
    sf::Texture screenTexture;
    screenTexture.create(SCREEN_WIDTH, SCREEN_HEIGHT);
    sf::Sprite screenSprite(screenTexture);

    double frequency = 5.0f;
    int octaves = 8;
    const siv::PerlinNoise perlin(time(0));
    const double fx = SCREEN_WIDTH / frequency;
    const double fy = SCREEN_HEIGHT / frequency;
    for(int i = 0; i < SCREEN_HEIGHT; i++) {
        for(int j = 0; j < SCREEN_WIDTH; j++) {
            cells[i][j] = Cell();
            cells[i][j].mass = DENSITY_CONSTANT * 1;
            cells[i][j].specificHeatCapacity = 4182;
            cells[i][j].temperature = (pow(MATH_E, perlin.accumulatedOctaveNoise2D_0_1(i / fx, j / fy, octaves) * 2.5f)) * 100;
        }
    }

    // for(int i = 0; i < SCREEN_HEIGHT - 150; i++) {
    //     for(int j = SCREEN_WIDTH / 2 - 100; j < SCREEN_WIDTH / 2 + 100; j++) {
    //         cells[i][j] = Cell();
    //         // cells[i][j].mass = DENSITY_CONSTANT * 7874;
    //         // cells[i][j].specificHeatCapacity = 440;
    //         cells[i][j].mass = DENSITY_CONSTANT * 1;
    //         cells[i][j].specificHeatCapacity = 4182;
    //         cells[i][j].temperature = 100;
    //     }
    // }

    std::vector<float> temperatures;
    std::vector<float> averages;
    std::vector<float> deltas;
    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            switch(event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }
                default: break;
            }

        }
        
        memcpy(newCells, cells, sizeof(Cell) * SCREEN_WIDTH * SCREEN_HEIGHT);
        for(int i = 0; i < SCREEN_HEIGHT; i++) {
            for(int j = 0; j < SCREEN_WIDTH; j++) {
                // Simulate on newCells with values from cells
                temperatures.push_back(cells[i][j].temperature);
                if(BOUND(CURRENT - SCREEN_WIDTH)) SIMULATE(i, j, i - 1, j);
                if(BOUND(CURRENT - SCREEN_WIDTH - 1)) SIMULATE(i, j, i - 1, j - 1);
                if(BOUND(CURRENT - SCREEN_WIDTH + 1)) SIMULATE(i, j, i - 1, j + 1);
                if(BOUND(CURRENT - 1)) SIMULATE(i, j, i, j - 1);
                if(BOUND(CURRENT + 1)) SIMULATE(i, j, i, j + 1);
                if(BOUND(CURRENT + SCREEN_WIDTH)) SIMULATE(i, j, i + 1, j);
                if(BOUND(CURRENT + SCREEN_WIDTH - 1)) SIMULATE(i, j, i + 1, j - 1);
                if(BOUND(CURRENT + SCREEN_WIDTH + 1)) SIMULATE(i, j, i + 1, j + 1);
                // draw Cells
                RGB color = tempToRGBA(cells[i][j].temperature, -300, 300);
                screenData[CURRENT * 4 + 0] = color.r;
                screenData[CURRENT * 4 + 1] = color.g;
                screenData[CURRENT * 4 + 2] = color.b;
                screenData[CURRENT * 4 + 3] = 255;
            }
        }
        // Swap "buffers"
        memcpy(cells, newCells, sizeof(Cell) * SCREEN_WIDTH * SCREEN_HEIGHT);

        screenTexture.update(screenData);
        averages.push_back(std::accumulate(temperatures.begin(), temperatures.end(), 0.0) / temperatures.size());
        deltas.push_back(temperatures[std::distance(temperatures.begin(), std::max_element(temperatures.begin(), temperatures.end()))]
                       - temperatures[std::distance(temperatures.begin(), std::min_element(temperatures.begin(), temperatures.end()))]);
        temperatures.clear();

        window.clear();
        // IMGUI stuff
        ImGui::SFML::Update(window, clock.restart());
        ImGui::Begin("Simulation Data");
        std::string averageT = "Average T: " + std::to_string(averages[averages.size() - 1]);
        ImGui::PlotLines(averageT.c_str(), averages.data(), averages.size());
        std::string largestDT = "Largest dT: " + std::to_string(deltas[deltas.size() - 1]);
        ImGui::PlotLines(largestDT.c_str(), deltas.data(), deltas.size());
        ImGui::End();
        
        window.draw(screenSprite);
        
        ImGui::SFML::Render(window);
        window.display();
    }

    return 0;
}