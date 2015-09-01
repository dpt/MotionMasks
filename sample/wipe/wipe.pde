int Y_AXIS = 1;
int X_AXIS = 2;

int max = 20;
int t = 0;
int softedge = 16;

void setup()
{
  size(320, 480, P2D);
  smooth(8);
}

void draw()
{
  background(0);
  
  if (t++ < max)
  {
    fill(255);
    noStroke();
    int w = 320 * t / (max - 1);
    rect(0, 0, w-softedge, 480);
    setGradient(w-softedge, 0, softedge, 480, 255, 0, X_AXIS);
    
    saveFrame("Wipe/####.png");
  }
  else
  {
    noLoop();
    t = 0;
  }
}

void setGradient(int x, int y, float w, float h, color c1, color c2, int axis ) {

  noFill();

  if (axis == Y_AXIS) {  // Top to bottom gradient
    for (int i = y; i <= y+h; i++) {
      float inter = map(i, y, y+h, 0, 1);
      color c = lerpColor(c1, c2, inter);
      stroke(c);
      line(x, i, x+w, i);
    }
  }  
  else if (axis == X_AXIS) {  // Left to right gradient
    for (int i = x; i <= x+w; i++) {
      float inter = map(i, x, x+w, 0, 1);
      color c = lerpColor(c1, c2, inter);
      stroke(c);
      line(i, y, i, y+h);
    }
  }
}
