int max = 20;
int t = 0;

void setup()
{
  size(320, 480, P2D);
  smooth(8);
}

void draw()
{
  background(255);
  
  if (t++ < max)
  {
    fill(0, 0, 0, 255 - 255 * t / (max - 1));
    noStroke();  
    rect(0, 0, 320, 480);
    
    saveFrame("Fade/####.png");
  }
  else
  {
    noLoop();
    t = 0;
  }
}
