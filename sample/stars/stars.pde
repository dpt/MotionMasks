class Star
{
  PShape s;
  float  x,y;
  float  scale;
  float  rotate;
  
  Star(float _x, float _y, float _scale, float _rotate) 
  {
    x = _x;
    y = _y;
    scale = _scale;
    
    s = createShape();
    s.fill(0);
    s.noStroke();
    s.vertex(0, -50);
    s.vertex(14, -20);
    s.vertex(47, -15);
    s.vertex(23, 7);
    s.vertex(29, 40);
    s.vertex(0, 25);
    s.vertex(-29, 40);
    s.vertex(-23, 7);
    s.vertex(-47, -15);
    s.vertex(-14, -20);
    s.end(CLOSE);
  }
  
  void display()
  {
    if (scale < 0)
      return;
    s.resetMatrix();
    s.rotate(rotate);
    s.scale(scale);
    shape(s, x, y);
  }
}

Star[] stars;
float a = 0;
int t = 0;

void setup()
{
  size(320, 480, P2D);
  smooth(8);
  
  stars = new Star[1];
  for (int i = 0; i < stars.length; i++)
    stars[i] = new Star(320 / 2, 480 / 2, 1, 0.01);
}

void draw()
{
  background(255);
  
  if (t++ < 20)
  {
    for (int i = 0; i < stars.length; i++)
    {
      stars[i].scale = 12 * (1.0001 + (sin(radians(a))));
      stars[i].rotate = 2 * (1 + (cos(radians(a))));
      stars[i].display();
    }
  
    a -= 4;
  
    saveFrame("output-####.png");
  }
  else
  {
    noLoop();
    a = 0;
    t = 0;
  }
}

