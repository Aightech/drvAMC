// C library headers
//#include "opencv2/opencv.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>

#define PI 3.14159265

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

        m_image_array[(posy * m_imageSize + posx) * 4 + 1] = 0;
        // const sf::Uint8 *ptr = m_image.getPixelsPtr();
        //   for(int i = 0; i < m_imageSize * m_imageSize * 4; i++)
        //       m_image_array[i] = *(ptr + i);

        const sf::Uint8 *ptr = m_image.getPixelsPtr();
        int cx = posx;
        int cy = posy;
        int h = m_imageSize;
        int w = m_imageSize;

        double rx = 50;
        double ry = rx;

        int distC = cx * cx + cy * cy;
        for(int i = 0; i < h; i++)
            for(int j = 0; j < w; j++)
            {
                int dist = sqrt((i - cy) * (i - cy) + (j - cx) * (j - cx));
                if(dist < rx && dist != 0)
                {
                    double coef = (1 + 10 * 0.0001 * (rx - dist));
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
    // uint32_t m_textureSize;
    // uint32_t m_textureOffset;
    // uint32_t m_textureSelect;

    std::vector<Material *> m_materials;
};

void
on_trackbar(int, void *)
{
}

int
main(int argc, char **argv)
{

    // Mat src = imread("foam.png", IMREAD_UNCHANGED);
    // Mat dest = src.clone();
    // Mat res = src.clone();

    // int w = src.cols;
    // int h = src.rows;

    // namedWindow("res", 1);

    // int g_slider = 0;       //slider pos value
    // int g_slider_max = 255; //slider max valu
    // createTrackbar("TrackbarName", "res", &g_slider, g_slider_max, on_trackbar);

    // int cx = w / 2;
    // int cy = h / 2;
    // double rx = cx;
    // double ry = cy;
    // int cc = cc;
    // createTrackbar("cc", "res", &cc, 100, on_trackbar);
    // createTrackbar("tx", "res", &cx, 600, on_trackbar);
    // createTrackbar("ty", "res", &cy, 600, on_trackbar);

    // for(;;)
    // {

    //     double kx = g_slider;
    //     double ky = kx;
    //     rx = kx * 10;
    //     ry = ky * 10;

    //     int distC = cx * cx + cy * cy;
    //     double d = 0; //cx - cx*(1+kx*distC);
    //     double c = 1; //-2*d/h;

    //     cout << d << " " << c << endl;
    //     bool inCircle;
    //     for(int i = 0; i < h; i++)
    //         for(int j = 0; j < h; j++)
    //         {
    //             inCircle = true;
    //             if(i < d || i > h - d || j < d || j > w - d)
    //             {
    //                 inCircle = false;
    //                 int dist = (i - cx) * (i - cx) + (j - cy) * (j - cy);
    //                 if(dist < distC * c * c)
    //                     inCircle = true; //res.at<Vec3b>(i, j) = Vec3b(0,0,0);
    //             }
    //             if(inCircle)
    //             {
    //                 //res.at<Vec3b>(i, j) = src.at<Vec3b>((i-d)/c, (j-d)/c);
    //                 int x_s = (i - d) / c;
    //                 int y_s = (j - d) / c;
    //                 int dist =
    //                     sqrt((x_s - cx) * (x_s - cx) + (y_s - cy) * (y_s - cy));
    //                 if(dist < rx && dist != 0)
    //                 {
    //                     double coef = (1 + cc * 0.0001 * (rx - dist));
    //                     int x = cx - double(cx - x_s) * coef;
    //                     int y = cy - double(cy - y_s) * coef;
    //                     x = max(0, min(h - 1, x));
    //                     y = max(0, min(w - 1, y));
    //                     res.at<Vec3b>(i, j) = src.at<Vec3b>(x, y);
    //                 }
    //                 else
    //                 {
    //                     res.at<Vec3b>(i, j) = src.at<Vec3b>(i, j);
    //                 }
    //             }
    //             else
    //                 res.at<Vec3b>(i, j) = src.at<Vec3b>(i, j);
    //         }

    // for (int y_s = 0; y_s < h; y_s++)
    // 	{
    // 	  for (int x_s = 0; x_s < w; x_s++)
    // 	    {
    // 	      int x = x_s;
    // 	      int y = y_s;
    // 	      int dist = (x_s-cx)*(x_s-cx)+(y_s-cy)*(y_s-cy);
    // 	      int i = cx+(x_s-cx)*(1+kx*dist);
    // 	      int j = cy+(y_s-cy)*(1+ky*dist);
    // 	      i = max(0, min(h-1, i));
    // 	      j = max(0, min(w-1, j));
    // 	      dest.at<Vec3b>(x_s, y_s) = src.at<Vec3b>(i, j);
    // 	    }
    // 	}

    //    imshow("res", res);
    //     if(waitKey(30) >= 0)
    //         break;
    // }

    //return 0;

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "SFML works!");
    TextureArr textarr;

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color(255, 255, 255, 255));
        textarr.update(window);
        textarr.draw(window);
        window.display();
    }
    return 0; // success
}

//FindFit[%, a/x^b+c, (sqrt(3) sqrt(607500 x^4 + x^3) - 1350 x^2)^(1/3)/(3^(2/3) x) - 1/(3^(1/3) (sqrt(3) sqrt(607500 x^4 + x^3) - 1350 x^2)^(1/3)) + 300,  {a,b,c}, x]

//f(x)=300+(1-400000 (0.0000006+k)) (x-300) (1+2 k (x-300)^(2))
