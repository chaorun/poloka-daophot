#include <cmath>
#include <cstdlib>
#include <ctime>

#include <poloka/reducedimage.h>
#include <poloka/daostar.h>
#include <poloka/daophotio.h>
#include <poloka/fileutils.h>
#include <poloka/fitsimage.h>

static void usage(const char* progname) {
  cerr << "Usage: " << progname << " <file with id ra dec mag> <dbimage>...<dbimage>\n"
       << "Create a star file planted.lst to be added with DAOPHOT/ADDSTAR\n"
       << "Input FILE format: ID RA DEC MAG\n\n";
  exit(EXIT_FAILURE);
}

struct genstar {
  string id;
  double ra,dec,mag;
};

static void read_generated(const char* filename, list<genstar>& stars) {
  ifstream in(filename);
  char c;
  while (in >> c) {
    in.unget();
    genstar star;
    in >> star.id >> star.ra >> star.dec >> star.mag;
    stars.push_back(star);    
  }
}


struct ImageAddStar {
  list<genstar> stars;

  ImageAddStar(const char* filename) {
    read_generated(filename, stars);
  }

  void operator () (const string& name) {
    ReducedImageRef rim = new ReducedImage(name);
    if (!rim->IsValid()) { 
      cerr << name << ": invalid dbimage\n";
      return;
    }

    GtransfoRef wcs = rim->RaDecToPixels();
    if (!wcs) {
      cerr << name << ": invalid wcs\n";
      return;
    }

    double zp = rim->AnyZeroPoint();
    Frame frame = rim->UsablePart();
    DaoStarList daostars;
    int i = 1;
    for (list<genstar>::iterator it=stars.begin(); it != stars.end(); ++it) {
      double x,y;
      wcs->apply(it->ra, it->dec, x, y);
      if (!frame.InFrame(x,y)) continue;
      DaoStar *daostar = new DaoStar;
      daostar->num = i++;
      daostar->x = x;
      daostar->y = y;
      daostar->flux = pow(10, 0.4*(zp - it->mag));
      daostar->sky = 0;
      daostars.push_back(daostar);
    }
    WriteDaoList(*rim, "planted.lst", daostars);
  }
};

int main( int nargs, char **args){

  if (nargs <=1) usage(args[0]);

  ImageAddStar imAddStar(args[1]);
  list<string> imList;

  for (int i=2; i < nargs; ++i)
    imList.push_back(args[i]);

  for_each(imList.begin(), imList.end(), imAddStar);

  return EXIT_SUCCESS;
}
