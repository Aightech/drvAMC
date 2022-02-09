#ifndef GUI_HPP
#define GUI_HPP

#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>

#include "drvAMC.hpp"

#define PI 3.14159265



class Interaction
{
    public:
    Interaction(){};
    double posx, posy, posz;
    double force = 0;
};

class Material
{
    public:
    Material(){};
    Material(std::string name, uint32_t stiff);

    void
    setSpritePos(uint32_t posx, uint32_t posy);

    void
    select(bool sel);

    void
    touch(Interaction &interaction);

    const sf::Sprite
    sprite()
    {
        return m_sprite;
    };
    const sf::IntRect &
    spriteRect()
    {
        return m_spriteRect;
    };

    bool m_selected = false;
    uint32_t m_stiffness;

    private:
    sf::Sprite m_sprite;
    sf::Texture m_texture;
    sf::Image m_image;
    sf::IntRect m_spriteRect;
    int32_t m_imageSelectOffset = 6;
    int32_t m_imageSizeX;
    int32_t m_imageSizeY;
    static std::string g_path;
    sf::Uint8 *m_image_array;
};

class TextureArr
{
    public:
    TextureArr(Driver *d);

    void
    draw(sf::RenderWindow &win);
    void
    update(sf::RenderWindow &win, Interaction &interaction);

    private:
    std::vector<Material *> m_materials;
    Driver *drv;
};


#endif
