#include <gui.hpp>
#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>

#include "drvAMC.hpp"

#define PI 3.14159265

std::string Material::g_path = "../fig/";

int
main(int argc, char **argv)
{

    Driver drv = Driver();
    drv.writeAccess();
    drv.enableBridge();

    drv.writeIndex<uint32_t>(0x38, 0x00, 10000);
    drv.writeIndex<int32_t>(0x45, 0x00, -5000);

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "DeepScreen");
    TextureArr textarr(&window, &drv);
    Interaction interaction;
    int32_t pos_measured_i32=0;

    while(window.isOpen())
    {

        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }
        try
        {
            pos_measured_i32 = drv.readIndex<int32_t>(0x12, 0x00);
            //std::cout << "pos: " << std::dec << pos_measured_i32 << std::endl;
        }
        catch(const char *msg)
        {
            std::cout << "ERROR: " << msg << "\n";
            break;
        }

        sf::Vector2i position = sf::Mouse::getPosition(window);
        interaction.posx = position.x;
        interaction.posy = position.y;
        interaction.posz = (5000 + pos_measured_i32) / 50.;

        if(interaction.posz < 0)
            interaction.posz = 0;
        // std::cout << "inter: " << std::dec << interaction.posz << "\n";
        window.clear(sf::Color(255, 255, 255, 255));
        textarr.update(window, interaction);
        textarr.draw(window);
        window.display();
    }

    drv.enableBridge(false);
    drv.writeAccess(false);
    return 0;
}
