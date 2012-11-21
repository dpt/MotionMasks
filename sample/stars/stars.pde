class Star
{
  PShape s;
  float  x,y;
  float  scale;
  
  Star(float _x, float _y, float _scale) 
  {
    x = _x;
    y = _y;
    scale = _scale;
    
    s = createShape();
    s.fill(255);
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
    s.scale(scale);
    shape(s, x, y);
  }
}

Star[] stars;
float a;

void setup()
{
  size(320, 480, P2D);
  smooth(8);
  
  stars = new Star[33];
  for (int i = 0; i < stars.length; i++)
    stars[i] = new Star(random(320), random(480), random(1));
}

int t = 0;

void draw()
{
  background(0);
  
  if (t++ < 10)
  {
    for (int i = 0; i < stars.length; i++)
    {
      stars[i].scale = 1.0001 + (cos(radians(a)));
      stars[i].display();
    }
  
    a += 20;
  
    saveFrame("output-####.png");
  }
  else
  {
    noLoop();
  }
}

