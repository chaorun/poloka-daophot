#include <iostream>
#include <fstream>
#include <fileutils.h>
#include <dictfile.h>
#include <basestar.h>
#include <frame.h>
#include <wcsutils.h>
#include <reducedimage.h>
#include <fitsimage.h>
#include <gtransfo.h>
#include <photstar.h>
#include <lightcurve.h>
#include <simfitphot.h>
#include <map>
#include <iomanip>

using namespace std;

static void usage(const char *pgname)
{
  cerr << pgname << " <dbimage1> <dbimage2> <dbimage3> ... -r <referenceimage> -c <catalog>" << endl ;
  cerr << "options:"<< endl;
  cerr << "     -o <catalog> : output catalog name (default is calibration.list)" << endl;
  exit(1);
}

static string getkey(const string& prefix, const DictFile& catalog) {
  if(!catalog.HasKey(prefix)) {
    cerr << "catalog does not not have info about " << prefix << endl;
    exit(2);
    return "absent";
  }
  return prefix;
}

class CalibratedStar : public BaseStar {
 public:
  CalibratedStar() {};
  CalibratedStar(BaseStar& toto) : BaseStar(toto) {};
  double ra,dec;
  double u,g,r,i,z,x,y;
  int id;
};

int main(int argc, char **argv)
{
  string referencedbimage = "";
  string catalogname = "";
  string matchedcatalogname = "calibration.list";
  vector<string> dbimages;
  
  if (argc < 7)  {usage(argv[0]);}
  for (int i=1; i<argc; ++i)
    {
      char *arg = argv[i];
      if (arg[0] != '-')
	{
	  dbimages.push_back(arg);continue;
	}
      switch (arg[1])
	{
	case 'r' : referencedbimage = argv[++i]; break;
	case 'c' : catalogname = argv[++i]; break;
	case 'o' : matchedcatalogname = argv[++i]; break;
	default : 
	  cerr << "unknown option " << arg << endl;
	  usage(argv[0]);
	}
    }
  

  if(!FileExists(catalogname)) {
    cerr << "cant find catalog " << catalogname << endl;
    usage(argv[0]);
  }
  cout << "catalog          = " << catalogname << endl;
  cout << "referencedbimage = " << referencedbimage << endl;
  cout << "n. dbimages      = " << dbimages.size() << endl;
  

  // put all of this info in a LightCurveList which is the food of the photometric fitter
  LightCurveList lclist;
  lclist.RefImage = new ReducedImage(referencedbimage);
  for (unsigned int im=0;im<dbimages.size();++im) {
    lclist.Images.push_back(new ReducedImage(dbimages[im]));
  }
  
  // we know want to put new objects in the list
  lclist.Objects.clear();
  lclist.clear();

  
  FitsHeader header(lclist.RefImage->FitsName());
  // prepare transfo and frames for stars' selection
  Frame W = lclist.RefImage->UsablePart();
  // W = W.Rescale(1.); // remove boundaries
  Gtransfo* Pix2RaDec=0;
  WCSFromHeader(header, Pix2RaDec);
  Gtransfo *RaDec2Pix = Pix2RaDec->InverseTransfo(0.01,W);
  Frame radecW = (W.ApplyTransfo(*Pix2RaDec)).Rescale(1.1);

  DictFile catalog(catalogname);
  
  
  int requiredlevel=3;
  // get keys for mag
  string band = header.KeyVal("TOADBAND");
  string mag_key=getkey("m"+band,catalog);
  string error_key=getkey("em"+band,catalog);
  string nmes_key=getkey("nmes"+band,catalog);
  
  BaseStar star;
  int count_total=0;
  int count_total_stars=0;
  int count_ok=0;
  
  double mag;
  char name[100];
  double mag_med;
  double mag_rms;
  map<RefStar*,CalibratedStar> assocs;
  
  for(DictFileCIterator entry=catalog.begin();entry!=catalog.end();++entry) {

    count_total++;
    
    if(entry->Value("level")!=requiredlevel)  continue; // not a star with correct level 
    mag=entry->Value(mag_key);
    
    count_total_stars++;
    
    star.x=entry->Value("x"); // ra (deg)
    star.y=entry->Value("y"); // dec (deg)
    
    // now check if star is in image
    if(!radecW.InFrame(star)) continue; // bye bye
    // apply transfo to this star (x,y)=pixels
    RaDec2Pix->apply(star,star);
    // check again
    if (!W.InFrame(star)) continue; // bye bye
    
    count_ok++;
    
    
    // set flux
    star.flux=pow(10.,-0.4*mag);
    
    // ok now we copy this star in a refstar and put it in the lclist.Objects
    CountedRef<RefStar> rstar = new RefStar(lclist.RefImage);
    sprintf(name,"calibstar%d",count_ok);
    rstar->name = name;
    rstar->type = 1; // a star
    rstar->band = band[0];
    rstar->x = star.x;
    rstar->y = star.y;
    rstar->ra = entry->Value("x"); // ra (deg)
    rstar->dec = entry->Value("y"); // dec (deg)
    rstar->jdmin = -1.e30; // always bright
    rstar->jdmax = 1.e30;
    lclist.Objects.push_back(rstar);
    
    // and also creat a lc (something stupid in the design)
    LightCurve lc(rstar);
    for (ReducedImageCIterator im=lclist.Images.begin(); im != lclist.Images.end(); ++im) {
      lc.push_back(*im, new PhotStar(star)); // add one image and one PhotStar
    }
    lclist.push_back(lc);
    
    // we also want to keep calibration info
    CalibratedStar cstar(star);
    cstar.ra=entry->Value("x");
    cstar.dec=entry->Value("y");
    cstar.u=entry->Value("mu");
    cstar.g=entry->Value("mg");
    cstar.r=entry->Value("mr");
    cstar.i=entry->Value("mi");
    cstar.z=entry->Value("mz");
    cstar.flux=star.flux;
    cstar.id=count_total;
    cstar.x=star.x;
    cstar.y=star.y;
    
    //keep a link between the two of them
    assocs[(RefStar*)rstar] = cstar;
    //if(count_ok>0)
    //break;
  }
  
  cout << count_total << " objects in the catalog" << endl;
  cout << count_total_stars << " correct stars (with mag in band " << band << ") in the catalog" << endl;
  cout << count_ok << " stars in this image" << endl;
  
  // ok now let's do the fit
  SimFitPhot doFit(lclist,false);
  doFit.dowrite=false; // don't write anything before all is done
  
  // does everything
  for_each(lclist.begin(), lclist.end(), doFit);
  
  // now we want to write many many things, let's make a list
  ofstream stream(matchedcatalogname.c_str());
  stream << "#x :" << endl;
  stream << "#y :" << endl;
  stream << "#flux :" << endl;
  stream << "#error :" << endl;
  stream << "#sky :" << endl;
  stream << "#skyerror :" << endl;
  stream << "#ra : initial " << endl;
  stream << "#dec : initial " << endl;
  stream << "#ix : initial x" << endl;
  stream << "#iy : initial y" << endl;
  stream << "#u : from catalog" << endl;
  stream << "#g : from catalog" << endl;
  stream << "#r : from catalog" << endl;
  stream << "#i : from catalog" << endl;
  stream << "#z : from catalog" << endl;
  stream << "#img : image number" << endl;
  stream << "#star : start number in the catalog" << endl;
  stream << "#end" <<endl;
  stream << setprecision(12);
  // first let's try to compute the ZP
  double weight;
  int count=0;
  double sumzp=0;
  double sumzp2=0;
  double sumweight=0;
  double zp,dzp;
  for(LightCurveList::iterator ilc = lclist.begin(); ilc!= lclist.end() ; ++ilc) { // loop on lc
    CalibratedStar cstar=assocs.find(ilc->Ref)->second;
    //cout << "=== " << cstar.r << " " << cstar.flux << " ===" << endl;
    int count_img=0;
    for (LightCurve::const_iterator it = ilc->begin(); it != ilc->end(); ++it) { // loop on points
      count_img++;
      const Fiducial<PhotStar> *fs = *it;
      if(fabs(fs->flux)<0.001) // do not print unfitted fluxes
	continue;
      stream << fs->x << " ";
      stream << fs->y << " ";
      stream << fs->flux << " ";
      if(fs->varflux>0)
	stream << sqrt(fs->varflux) << " ";
      else
	stream << 0 << " ";
      stream << fs->sky << " ";
      if(fs->varsky>0 && fs->varsky<10000000.)
	stream << sqrt(fs->varsky) << " ";
      else
	stream << 0 << " ";
      stream << cstar.ra << " ";
      stream << cstar.dec << " ";
      stream << cstar.x << " ";
      stream << cstar.y << " ";
      stream << cstar.u << " ";
      stream << cstar.g << " ";
      stream << cstar.r << " ";
      stream << cstar.i << " ";
      stream << cstar.z << " ";
      stream << count_img << " "; 
      stream << cstar.id << " ";     
      stream << endl;
      
      if(fs->flux<=0) continue;
      dzp = sqrt(fs->varflux)/fs->flux;
      if(dzp < 0.002)  dzp = 0.002; // minimum error
      if(dzp>0.5) continue;
      
      weight = 1./dzp/dzp;
      count ++;
      sumweight += weight;
      zp = 2.5*log10(fs->flux/cstar.flux);
      sumzp += zp*weight;
      sumzp2 += zp*zp*weight;
    }
  }
  stream.close();
  
  zp = sumzp/sumweight;
  double rms = sqrt(sumzp2/sumweight -zp*zp);

  FILE *file = fopen("calibration.summary","w");
  fprintf(file,"BEFORE_ROBUSTIFICATION_zp_rms_count= %6.6f %6.6f %d\n",zp,rms,count);
  
  // robustify
  double zpmin = zp - rms*3;
  double zpmax = zp + rms*3;
  
  count=0;
  sumzp=0;
  sumzp2=0;
  sumweight=0;
  
  for(LightCurveList::iterator ilc = lclist.begin(); ilc!= lclist.end() ; ++ilc) { // loop on lc
    CalibratedStar cstar=assocs.find(ilc->Ref)->second;
    for (LightCurve::const_iterator it = ilc->begin(); it != ilc->end(); ++it) { // loop on points
      const Fiducial<PhotStar> *fs = *it;
      if(fabs(fs->flux)<0.001) // do not print unfitted fluxes
	continue;
      if(fs->flux<=0) continue;
      dzp = sqrt(fs->varflux)/fs->flux;
      if(dzp < 0.002)  dzp = 0.002; // minimum error
      if(dzp>0.5) continue;
      zp = 2.5*log10(fs->flux/cstar.flux);
      if(zp<zpmin) continue;
      if(zp>zpmax) continue;
      

      weight = 1./dzp/dzp;
      count ++;
      sumweight += weight;
      sumzp += zp*weight;
      sumzp2 += zp*zp*weight;
    }
  }
  
  zp = sumzp/sumweight;
  rms = sqrt(sumzp2/sumweight -zp*zp);
  fprintf(file,"RESULT_zp_rms_count= %6.6f %6.6f %d\n",zp,rms,count);


  fclose(file);

  return EXIT_SUCCESS;
}

