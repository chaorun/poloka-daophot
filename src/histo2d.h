#ifndef HISTO2D__H
#define HISTO2D__H

#include "persistence.h"

class Histo2d {
  CLASS_VERSION(Histo2d,1);
  #define Histo2d__is__persistent
 private:
  float *data;
  int nx,ny;
  float minx,miny;
  float scalex, scaley;

 public:
  Histo2d() {}
    Histo2d(int nx, float minx, float maxx, int ny, float miny,float maxy);
    void Fill(float x, float y, float weight=1.);
    double  MaxBin(double &x, double &y);
    void BinWidth(double &Hdx, double &Hdy) { Hdx = 1./scalex; Hdy = 1./scaley;}
    void ZeroBins(double &X, double &Y);

    ~Histo2d() { delete [] data;}
};

#endif /* HISTO2D__H */
