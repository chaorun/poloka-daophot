#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>

#include "allreducedimage.h"
#include "reducedimage.h"
#include "fastfinder.h"
#include "detection.h"
#include "sestar.h"
#include "fileutils.h"

static void usage(const char* prog) {
  cerr << prog << " GENFILE DBIMAGE...\n";
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

struct MatchDetStar {

  list<genstar> stars;
  double maxDist;

  MatchDetStar(const char* filename) : maxDist(5) {
    read_generated(filename, stars);
    cout << " read " <<  stars.size() << " stars" << endl;
  }

  void operator () (const string& name) {

    ReducedImageRef im = ReducedImageNew(name);
    if (!im->IsValid()) { 
      cerr << " not a valid dbimage: " << name << endl;
      return;
    }
    
    GtransfoRef wcs = im->RaDecToPixels();
    DetectionList starList(im->Dir()+"/det.list");
    //SEStarList starList(im->Dir()+"/se.list");
    BaseStar zeroStar;
    FastFinder finder(*Detection2Base(&starList));
    //FastFinder finder(*SE2Base(&starList));
    ofstream out((im->Dir()+"detplant.list").c_str());
    ofstream outds9((im->Dir()+"detplant.reg").c_str());
    Frame frame = im->UsablePart();
    for (list<genstar>::const_iterator it=stars.begin(); it != stars.end(); ++it) {
      double x,y;
      wcs->apply(it->ra, it->dec, x, y);
      if (!frame.InFrame(x,y)) continue;
      const BaseStar* detStar = finder.FindClosest(Point(x,y), maxDist);
      if (detStar) {
	out << it->id << ' ' << it->ra << ' ' << it->dec << ' ' << it->mag
	    << ' ' << detStar->flux/detStar->eflux << endl;
	outds9 << "circle(" << x << "," << y << ",10p) # text = {" << it->mag << "}\n";
      } else {
	out << it->id << ' ' << it->ra << ' ' << it->dec << ' ' << it->mag 
	    << " 0" << endl;
	outds9 << "circle(" << x << "," << y << ",10p) # color = cyan text = {" << it->mag << "}\n";
      }
    }
  }
};

int main( int nargs, char **args){
  if (nargs <=1) usage(args[0]);

  MatchDetStar matchStar(args[1]);
  list<string> imList;
  for (int i=2; i < nargs; ++i)
    imList.push_back(args[i]);

  for_each(imList.begin(), imList.end(), matchStar);

  return EXIT_SUCCESS;
}
