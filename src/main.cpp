// C library headers
//#include "opencv2/opencv.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>

#include "drvAMC.hpp"

#define PI 3.14159265

int wheel = 0;

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
    Material(std::string name, uint32_t stiff)
    {
        m_image.loadFromFile(g_path + name + ".png");
        m_imageSizeX = m_image.getSize().x;
        m_imageSizeY = m_image.getSize().y;

        m_texture.loadFromImage(m_image);
        m_sprite.setTexture(m_texture);

        m_spriteRect =
            sf::IntRect(sf::IntRect(m_imageSelectOffset, m_imageSelectOffset,
                                    m_imageSizeX - 2 * m_imageSelectOffset,
                                    m_imageSizeY - 2 * m_imageSelectOffset));
        m_sprite.setTextureRect(m_spriteRect);

        m_image_array = new sf::Uint8[m_imageSizeX * m_imageSizeY * 4];
        const sf::Uint8 *ptr = m_image.getPixelsPtr();
        for(int i = 0; i < m_imageSizeX * m_imageSizeY * 4; i++)
            m_image_array[i] = *(ptr + i);
        //std::cout << m_image.getSize().x << std::endl;
        m_stiffness = stiff;
    };

    void
    setSpritePos(uint32_t posx, uint32_t posy)
    {
        m_sprite.setPosition(sf::Vector2f(posx, posy));
        m_spriteRect.left = posx;
        m_spriteRect.top = posy;
    };

    void
    select(bool sel)
    {
        if(sel && !m_selected)
        {
            m_selected = true;
            sf::IntRect rect = m_sprite.getTextureRect();
            rect.left -= m_imageSelectOffset;
            rect.top -= m_imageSelectOffset;
            rect.width += 2 * m_imageSelectOffset;
            rect.height += 2 * m_imageSelectOffset;
            m_sprite.setTextureRect(rect);
            m_sprite.move(
                sf::Vector2f(-m_imageSelectOffset, -m_imageSelectOffset));
        }
        else if(!sel && m_selected)
        {
            m_selected = false;
            sf::IntRect rect = m_sprite.getTextureRect();
            rect.left += m_imageSelectOffset;
            rect.top += m_imageSelectOffset;
            rect.width -= 2 * m_imageSelectOffset;
            rect.height -= 2 * m_imageSelectOffset;
            m_sprite.setTextureRect(rect);
            m_sprite.move(
                sf::Vector2f(m_imageSelectOffset, m_imageSelectOffset));
        }
    };

    void
    touch(Interaction &interaction)
    {
        uint32_t cx = interaction.posx;
        uint32_t cy = interaction.posy;
        uint32_t posz = interaction.posz;
        const sf::Uint8 *ptr = m_image.getPixelsPtr();
        double radius = posz;

        auto mapping = [radius](int d, int c, int ij, int max) {
            double coef = 1 + 0.002 * (radius - d);
            ij = c - (c - ij) * coef;
            return std::max(0, std::min(max - 1, ij));
        };

        for(int i = 0; i < m_imageSizeY; i++)
            for(int j = 0; j < m_imageSizeX; j++)
            {
                int x = j; //by default the map maping is identical
                int y = i;
                int dist = sqrt((i - cy) * (i - cy) + (j - cx) * (j - cx));
                if(dist < radius && dist != 0) //if the pix is in the circle
                {
                    x = mapping(dist, cx, j, m_imageSizeX);
                    y = mapping(dist, cy, i, m_imageSizeY);
                }
                for(int k : {0, 1, 2, 3}) //set the pix to its mapped orignal
                    m_image_array[(i * m_imageSizeY + j) * 4 + k] =
                        *(ptr + (y * m_imageSizeY + x) * 4 + k);
            }
        m_texture.update(m_image_array);
    };

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
    TextureArr(Driver *d)
    {
        drv = d;
        std::string listMat[] = {"sand", "gelee", "square", "foam"};
        for(int i = 0; i < 4; i++)
            m_materials.push_back(new Material(listMat[i], 10000 * i+10000));

        for(int i = 0; i < 2; i++)
            for(int j = 0; j < 2; j++)
                m_materials[i * 2 + j]->setSpritePos(10 + 400 * i,
                                                     10 + 400 * j);
    };

    void
    draw(sf::RenderWindow &win)
    {
        for(auto mat : m_materials) win.draw(mat->sprite());
    }
    void
    update(sf::RenderWindow &win, Interaction &interaction)
    {
        // get the local mouse position (relative to a window)
        sf::Vector2i pos(interaction.posx, interaction.posy);
        for(auto mat : m_materials)
        {
            const sf::IntRect rect = mat->spriteRect();
            bool selected = rect.contains(static_cast<sf::Vector2i>(pos));
            if(selected == true && mat->m_selected == false)
	      {
                drv->writeIndex<uint32_t>(0x38, 0x00, mat->m_stiffness);
		std::cout << mat->m_stiffness << std::endl;
	      }
            
            mat->select(selected);
            if(selected)
            {
                Interaction interacRelative;
                interacRelative.posx = interaction.posx - rect.left;
                interacRelative.posy = interaction.posy - rect.top;
                interacRelative.posz = interaction.posz;
                mat->touch(interacRelative);
            }
        }
    }

    private:
    std::vector<Material *> m_materials;
    Driver *drv;
};

std::string Material::g_path = "../fig/";

int
main(int argc, char **argv)
{

    Driver drv = Driver("/dev/ttyUSB0");
    // for(;;)
    // {
    //   try
    // 	{
    // 	  int32_t pos_measured_i32 = drv.readIndex<int32_t>(0x12, 0x00);
    // 	  std::cout << "pos: " << std::dec << pos_measured_i32 << "\n";
    // 	}
    //   catch (const char* msg)
    // 	{
    // 	  std::cout << "ERROR: " << msg << "\n";
    // 	  exit(0);
    // 	}
    // }
    // uint16_t v;
    drv.writeAccess();
    drv.enableBridge();

    drv.writeIndex<uint32_t>(0x38, 0x00, 10000);
    drv.writeIndex<int32_t>(0x45, 0x00, -5000);

    // // for(;;)
    //       {
    // int32_t pos_measured_i32 = drv.readIndex<int32_t>(0x12, 0x00);
    //       std::cout << "pos: " << std::dec << pos_measured_i32 << "\n";
    //       }

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "SFML works!");
    TextureArr textarr(&drv);
    Interaction interaction;
    int32_t pos_measured_i32;

    while(window.isOpen())
    {
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
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            // else if(event.type == sf::Event::MouseWheelMoved)
            //     interaction.posz += 4 * event.mouseWheel.delta;
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
