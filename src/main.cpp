// C library headers
#include <iostream>
#include <string.h>
#include <vector>
#include <math.h> 
#include <SFML/Graphics.hpp>
#include "opencv2/opencv.hpp"

#define PI 3.14159265

class Material
{
public:
  Material(){};
  Material(sf::Texture& texture,uint32_t posx, uint32_t posy, uint32_t size, uint32_t selectOffset)
  {
    setImage(texture, posx, posy, size, selectOffset);
  };
  void setImage(sf::Texture& texture, uint32_t posx, uint32_t posy, uint32_t size, uint32_t selectOffset)
  {
    m_imageSelectOffset = selectOffset;
    m_sprite.setTexture(texture);
    m_sprite.setTextureRect(sf::IntRect(posx, posy, size, size));
    m_spriteRect = sf::IntRect(posx, posy, size, size);
    m_sprite.setPosition(sf::Vector2f(posx,posy));
  };
  void mvImage(sf::Vector2f pos)
  {
    m_sprite.setPosition(pos);
  };

  void select(bool sel)
  {
    if(sel && !m_selected)
      {	
	m_selected=true;
	sf::IntRect rect = m_sprite.getTextureRect();   
	rect.left  -= m_imageSelectOffset;
	rect.top   -= m_imageSelectOffset;	
	rect.width += 2*m_imageSelectOffset;
	rect.height  += 2*m_imageSelectOffset;
	m_sprite.setTextureRect(rect);
	m_sprite.move(sf::Vector2f(-m_imageSelectOffset, -m_imageSelectOffset));
      }
    else if(!sel && m_selected)
      {
	m_selected=false;
	sf::IntRect rect = m_sprite.getTextureRect();
	rect.left  += m_imageSelectOffset;
	rect.top   += m_imageSelectOffset;	
	rect.width -= 2*m_imageSelectOffset;
	rect.height -= 2*m_imageSelectOffset;
	m_sprite.setTextureRect(rect);
	m_sprite.move(sf::Vector2f(m_imageSelectOffset, m_imageSelectOffset));
      }
  };

  const sf::Sprite sprite() {return m_sprite;};
  const sf::IntRect& spriteRect() {return m_spriteRect;};
  
private:
  sf::Sprite m_sprite;
  sf::IntRect m_spriteRect;
  int32_t m_imageSelectOffset;
  
  bool m_selected=false;
};

class TextureArr
{
public:
  TextureArr()
  {
    if (!m_texture.loadFromFile("texture.png"))
      {
	// error...
      } 
    m_texture.setSmooth(true);
    for(int i = 0; i<2;i++)
      for(int j = 0; j<2; j++)
	m_materials.push_back(new Material(m_texture,17+376*i, 17+376*j, 376, 4));
    
  };

  void draw(sf::RenderWindow& win)
  {
    for(auto mat : m_materials)
      win.draw(mat->sprite());
  }
  void update(sf::RenderWindow& win)
  {
    // get the local mouse position (relative to a window)
    sf::Vector2i pos = sf::Mouse::getPosition(win); // window is a sf::Window
    for(auto mat : m_materials)
      mat->select(mat->spriteRect().contains(static_cast<sf::Vector2i>(sf::Mouse::getPosition(win))));
  }

  
private:
  uint32_t m_textureSize;
  uint32_t m_textureOffset;
  uint32_t m_textureSelect;
  
  sf::Texture m_texture;
  std::vector<Material*> m_materials;
  
};

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
using namespace cv;
using namespace std;
string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
  case CV_8U:  r = "8U"; break;
  case CV_8S:  r = "8S"; break;
  case CV_16U: r = "16U"; break;
  case CV_16S: r = "16S"; break;
  case CV_32S: r = "32S"; break;
  case CV_32F: r = "32F"; break;
  case CV_64F: r = "64F"; break;
  default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}



void on_trackbar(int, void*)
{
}



int main(int argc, char** argv )
{
  
  Mat src = imread("foam.png",IMREAD_UNCHANGED);
  Mat dest= src.clone();
  Mat res = src.clone();
  
  int w = src.cols;
  int h = src.rows;

  namedWindow("res", 1);
  
  int g_slider=0; //slider pos value
  int g_slider_max=255; //slider max valu
  createTrackbar("TrackbarName", "res", &g_slider, g_slider_max, on_trackbar);

  
      
  int cx=w/2;
  int cy=h/2;
  double rx=cx;
  double ry=cy;
  int  cc=cc;
  createTrackbar("cc", "res", &cc, 100, on_trackbar);
  createTrackbar("tx", "res", &cx, 600, on_trackbar);
  createTrackbar("ty", "res", &cy, 600, on_trackbar);

  for (;;)
    {
      
      double kx=g_slider;
      double ky=kx;
      rx=kx*10;
      ry=ky*10;

      int distC = cx*cx+cy*cy;
      double d = 0;//cx - cx*(1+kx*distC);
      double c = 1;//-2*d/h;
      
      cout << d << " "  << c << endl;
      bool  inCircle; 
      for(int i=0; i<h; i++)
      	for(int j=0; j<h; j++)
      	  {
	    inCircle = true; 
	    if(i<d || i >h-d || j < d || j > w - d)
	      {
		inCircle = false; 
		int dist = (i-cx)*(i-cx)+(j-cy)*(j-cy);
		if(dist < distC*c*c)
		  inCircle = true; //res.at<Vec3b>(i, j) = Vec3b(0,0,0);
	      }
	    if(inCircle)
	      {
		//res.at<Vec3b>(i, j) = src.at<Vec3b>((i-d)/c, (j-d)/c);
		int x_s = (i-d)/c;
		int y_s = (j-d)/c; 
		int dist = sqrt((x_s-cx)*(x_s-cx)+(y_s-cy)*(y_s-cy));
		if(dist < rx && dist!=0)
		  {
		    double coef = (1+cc*0.0001*(rx-dist));
		    int x = cx-double(cx-x_s)*coef;
		    int y = cy-double(cy-y_s)*coef;
		    x  = max(0, min(h-1, x));
		    y = max(0, min(w-1, y));
		    res.at<Vec3b>(i, j)  = src.at<Vec3b>(x, y);
		  }
		else
		  {
		    res.at<Vec3b>(i, j)  = src.at<Vec3b>(i, j);
		  }
	      }
	    else
	      res.at<Vec3b>(i, j)  = src.at<Vec3b>(i, j);
	      
      	  }
      
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
      
      
      imshow( "res", res );
      if (waitKey(30) >= 0)
	break;
    }
  
  return 0;
  
  // sf::RenderWindow window(sf::VideoMode(1000, 1000), "SFML works!");
  // TextureArr textarr;
  
  //   while (window.isOpen())
  //   {
  //     sf::Event event;
  //     while (window.pollEvent(event))
  //       {
  // 	  if (event.type == sf::Event::Closed)
  // 	    window.close();
  //       }
      
  //     window.clear( sf::Color(255, 255, 255, 255));
  //     textarr.update(window);
  //     textarr.draw(window);
  //     window.display();
	
  //   }
  return 0; // success
}

//FindFit[%, a/x^b+c, (sqrt(3) sqrt(607500 x^4 + x^3) - 1350 x^2)^(1/3)/(3^(2/3) x) - 1/(3^(1/3) (sqrt(3) sqrt(607500 x^4 + x^3) - 1350 x^2)^(1/3)) + 300,  {a,b,c}, x]

//f(x)=300+(1-400000 (0.0000006+k)) (x-300) (1+2 k (x-300)^(2))
