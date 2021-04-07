// C library headers
//#include "opencv2/opencv.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>

#define PI 3.14159265

int wheel =0;

class Material
{
    public:
    Material(){};
    Material(std::string name)
    {
        m_image.loadFromFile(name + ".png");
        m_texture.loadFromImage(m_image);
        m_sprite.setTexture(m_texture);

        m_spriteRect =
            sf::IntRect(sf::IntRect(m_imageSelectOffset, m_imageSelectOffset,
                                    m_imageSize - 2 * m_imageSelectOffset,
                                    m_imageSize - 2 * m_imageSelectOffset));
        m_sprite.setTextureRect(m_spriteRect);
        m_sprite.setPosition(sf::Vector2f(4, 4));
        const sf::Uint8 *ptr = m_image.getPixelsPtr();
        for(int i = 0; i < m_imageSize * m_imageSize * 4; i++)
            m_image_array[i] = *(ptr + i);
        std::cout << m_image.getSize().x << std::endl;
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
    touch(uint32_t posx, uint32_t posy, uint32_t posz)
    {
        const sf::Uint8 *ptr = m_image.getPixelsPtr();
        int cx = posx;
        int cy = posy;
        int h = m_imageSize;
        int w = m_imageSize;

        double rx = wheel*4;
        double ry = rx;

        int distC = cx * cx + cy * cy;
        for(int i = 0; i < h; i++)
            for(int j = 0; j < w; j++)
            {
                int dist = sqrt((i - cy) * (i - cy) + (j - cx) * (j - cx));
                if(dist < rx && dist != 0)
                {
                    double coef = (1 + 20 * 0.0001 * (rx - dist));
                    int x = cx - double(cx - j) * coef;
                    int y = cy - double(cy - i) * coef;
                    x = std::max(0, std::min(h - 1, x));
                    y = std::max(0, std::min(w - 1, y));
                    //res.at<Vec3b>(i, j) = src.at<Vec3b>(x, y);

                    m_image_array[(i * m_imageSize + j) * 4 + 0] =
                        *(ptr + (y * m_imageSize + x) * 4 + 0);
                    m_image_array[(i * m_imageSize + j) * 4 + 1] =
                        *(ptr + (y * m_imageSize + x) * 4 + 1);
                    m_image_array[(i * m_imageSize + j) * 4 + 2] =
                        *(ptr + (y * m_imageSize + x) * 4 + 2);
                    m_image_array[(i * m_imageSize + j) * 4 + 3] =
                        *(ptr + (y * m_imageSize + x) * 4 + 3);
                }
                else
                {
                    m_image_array[(i * m_imageSize + j) * 4 + 0] =
                        *(ptr + (i * m_imageSize + j) * 4 + 0);
                    m_image_array[(i * m_imageSize + j) * 4 + 1] =
                        *(ptr + (i * m_imageSize + j) * 4 + 1);
                    m_image_array[(i * m_imageSize + j) * 4 + 2] =
                        *(ptr + (i * m_imageSize + j) * 4 + 2);
                    m_image_array[(i * m_imageSize + j) * 4 + 3] =
                        *(ptr + (i * m_imageSize + j) * 4 + 3);
                }
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

    private:
    sf::Sprite m_sprite;
    sf::Texture m_texture;
    sf::Image m_image;
    sf::IntRect m_spriteRect;
    int32_t m_imageSelectOffset = 6;
    static const int32_t m_imageSize = 389;
    sf::Uint8 m_image_array[m_imageSize * m_imageSize * 4];

    bool m_selected = false;
};

class TextureArr
{
    public:
    TextureArr()
    {
        std::string listMat[] = {"foam"};
        for(int i = 0; i < 2; i++)
            for(int j = 0; j < 2; j++)
                m_materials.push_back(new Material(listMat[0]));

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
    update(sf::RenderWindow &win)
    {
        // get the local mouse position (relative to a window)
        sf::Vector2i pos =
            sf::Mouse::getPosition(win); // window is a sf::Window
        for(auto mat : m_materials)
        {
            const sf::IntRect rect = mat->spriteRect();
            bool selected = rect.contains(static_cast<sf::Vector2i>(pos));
            mat->select(selected);
            if(selected)
            {
                mat->touch(pos.x - rect.left, pos.y - rect.top, 5);
            }
        }
    }

    private:
    std::vector<Material *> m_materials;
};


int
main(int argc, char **argv)
{

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "SFML works!");
    TextureArr textarr;

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
	    else if(event.type == sf::Event::MouseWheelMoved)
            {
	      wheel += event.mouseWheel.delta;
                // display number of ticks mouse wheel has moved
                std::cout << wheel << '\n';
            }
        }

        window.clear(sf::Color(255, 255, 255, 255));
        textarr.update(window);
        textarr.draw(window);
        window.display();
    }
    return 0;
}

