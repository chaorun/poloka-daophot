#include <iostream>
#include <fstream>
#include <fileutils.h>
#include <dicstar.h>
#include <basestar.h>
#include <gaussianfit.h>

#include <map>
#include <iomanip>

using namespace std;

static void usage(const char *pgname)
{
  cerr << pgname << " -c <dicstarlist>" << endl;
  cerr << "options:  -m <keyofreferencemagnitude> (default is \"mag\")"<< endl;
  cerr << "          -f <keyofflux>               (default is \"flux\")" << endl ;
  cerr << "          -o <output ASCII file>       (default is zpfitter.dat)" << endl ;
  cerr << "          -d mjd_min mjd_max       (date range)" << endl ;
  
  
  exit(1);
}

static double sqr(const double& x) {return x*x;};

//////////////////////////////////////////////////////
// ZPSTAR
//////////////////////////////////////////////////////
class zpstar {
private :
  double sum_1;
  double sum_w;
  double sum_fw;
  double sum_s;
  double sum_f2w;
  double sum_w01;
  double sum_fw01;
  double sum_f2w01;
  
public :
  double init_ra,init_dec;
  double init_x,init_y;  
  double fitted_ra,fitted_dec;
  double fitted_x,fitted_y;
  double mag,error;
  double psfchi2,zpchi2,zpchi2_01;
  double nmag,ndist;
  double mag_min,mag_max;
  double g,ge,r,re,i,ie,z,ze;
  double catalog_mag;
  double catalog_mage;
  double zp;
  double sky;
  double flux;
  double fluxrms;
  int satur;
  int contam;
  int var;
  int nostar;
  int id;
  
  // not saved in file:
  double neic;
  double flux_min,flux_max;
  
  int nmeas() const {
    return int(sum_1);
  }

  void reset() {
    id = -1;
    sum_1 = 0;
    sum_w = 0;
    sum_fw = 0;
    sum_f2w = 0;
    sum_w01 = 0;
    sum_fw01 = 0;
    sum_f2w01 = 0;
    sum_s = 0;
    mag = -1;
    error = -1;
    init_ra = 0;
    init_dec = 0;
    fitted_ra = 0;
    fitted_dec = 0;
    init_x = 0;
    init_y = 0;
    fitted_x = 0;
    fitted_y = 0;
    satur = 0;
    contam = 0;
    var = 0;
    nostar = 0;
    flux_min = 1.e30;
    flux_max = -1.e30;
    
  }

  void load(const DicStar* istar) {

    id = int(istar->getval("star"));

    init_ra = istar->getval("ra");
    init_dec = istar->getval("dec");
    init_x = istar->getval("ix");
    init_y = istar->getval("iy");
    fitted_x = istar->getval("x");
    fitted_y = istar->getval("y");
    
    psfchi2 = istar->getval("chi2pdf");
    neic = istar->getval("neic"); 
    ndist = istar->getval("neid"); 

    g = istar->getval("g");
    ge = istar->getval("ge");
    r = istar->getval("r");
    re = istar->getval("re");
    i = istar->getval("i");
    ie = istar->getval("ie");
    z = istar->getval("z");
    ze = istar->getval("ze");  
    catalog_mag = istar->getval("mag");
    catalog_mage= istar->getval("mage");
    
    double w = 1./sqr(istar->getval("error"));
    flux = istar->flux;
		     
    sum_1 += 1;
    sum_w += w;
    sum_fw += flux*w;
    sum_f2w += flux*flux*w;
    sum_s += istar->getval("sky");
    
    if(flux>0) { // why requiring (flux>0) ?? P.A.
      double w01 = 1./(sqr(istar->getval("error"))+sqr(0.01*istar->flux));
      sum_w01 += w01;
      sum_fw01 += flux*w01;
      sum_f2w01 += flux*flux*w01;
    }
    
    if(istar->getval("satur")>0.5) satur = 1;
    
    if(flux<flux_min)
      flux_min = flux;
    if(flux>flux_max)
      flux_max = flux;
    
  }
  
  void process() {
    flux = sum_fw/sum_w;
    error = 1./sqrt(sum_w); // flux
    fluxrms = sum_f2w/sum_w-flux*flux;
    fluxrms = (fluxrms>0) ? sqrt(fluxrms) : -1;
    error = error/flux*2.5/log(10.); // mag
    mag   = -2.5*log10(flux)+zp;
    mag_min = -2.5*log10(flux_max)+zp;
    mag_max = -2.5*log10(flux_min)+zp;
    sky = sum_s/sum_1;
    
    zpchi2 = (sum_f2w + sum_w*sqr(flux) - 2.*sum_fw*flux)/(sum_1-1.);
    if(zpchi2>1)
      error *= sqrt(zpchi2);
    
    zpchi2_01 = (sum_f2w01 + sum_w01*sqr(flux) - 2.*sum_fw01*flux)/(sum_1-1.);
    
    // scores
    nmag = neic/flux*2.5/log(10.);
    if(nmag>0.005)
      contam = 1;
    
    if(zpchi2_01>5)
      var = 1;
    
    if(psfchi2>100)
      nostar = 1;
  }
  
  zpstar() {
    zp=0;
    reset();
  }
  
  void write(ostream &s) const {
    
    s << init_ra << " ";
    s << init_dec << " ";
    s << fitted_ra  << " ";
    s << fitted_dec << " ";
    s << init_x << " ";
    s << init_y << " ";
    s << fitted_x << " ";
    s << fitted_y << " ";
    s << id << " ";
    s << mag << " ";
    s << error << " ";
    s << flux << " ";
    s << fluxrms << " ";
    s << sum_1 << " ";
    s << zpchi2 << " ";
    s << zpchi2_01 << " ";
    s << nmag << " ";
    s << ndist << " ";
    s << mag_min << " ";
    s << mag_max << " ";
    s << sky << " ";
    s << psfchi2 << " ";
    s << satur << " ";
    s << contam << " ";
    s << var << " ";
    s << nostar << " ";
    s << g << " ";
    s << ge << " ";
    s << r << " ";
    s << re << " ";
    s << i << " ";
    s << ie << " ";
    s << z << " ";
    s << ze << " ";
  }
  
  friend ostream& operator << (ostream &stream, const zpstar &s)
  { s.write(stream); return stream;}
};


//////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////
int main(int argc, char **argv)
{
  
  string catalogname = "";
  string magkey = "mag";
  string fluxkey = "flux";
  string outputfilename = "zpfitter.dat";
  double mjd_min = 0;
  double mjd_max = 1.e12;
  
  if (argc < 3)  {usage(argv[0]);}
  for (int i=1; i<argc; ++i)
    {
      char *arg = argv[i];
      if (arg[0] != '-')
	{
	  cerr << "unexpected parameter " << arg << endl;
	  usage(argv[0]);
	}
      switch (arg[1])
	{
	case 'm' : magkey = argv[++i]; break;
	case 'c' : catalogname = argv[++i]; break;
	case 'f' : fluxkey = argv[++i]; break;
	case 'o' : outputfilename = argv[++i]; break;
	case 'd' : mjd_min = atof(argv[++i]); mjd_max = atof(argv[++i]);  break;
	default : 
	  cerr << "unknown option " << arg << endl;
	  usage(argv[0]);
	}
    }
  
  
  if(!FileExists(catalogname)) {
    cerr << "cant find catalog " << catalogname << endl;
    usage(argv[0]);
  }
  
  DicStarList catalog(catalogname);
  if ( catalog.empty()) {
    cerr << "catalog is empty" << endl;
    usage(argv[0]);
  }
  int nstars=-1;
  if(catalog.GlobVal().HasKey("NSTARS")) nstars=int(catalog.GlobVal().getDoubleValue("NSTARS"));
  int nimages=-1;
  if(catalog.GlobVal().HasKey("NIMAGES")) nimages=int(catalog.GlobVal().getDoubleValue("NIMAGES"));
  
  // fill a vector of ZPs
  unsigned int nentries = catalog.size();
  double* values = new double[nentries];
  double flux,mag;
  
  float contamination_cut = 0.005;
  double mjd;
  int count = 0;
  for(DicStarCIterator entry=catalog.begin();entry!=catalog.end();++entry) {
    
    if(mjd_min>0) {
      mjd = (*entry)->getval("mjd");
      if (mjd<mjd_min) continue;
      if (mjd>mjd_max) continue;
    }

    if ( (*entry)->getval("satur") >0.5 ) // cause has saturated pixels
      continue;
    
    if(fluxkey=="flux")
      flux = (*entry)->flux;
    else
      flux = (*entry)->getval(fluxkey);
    mag = (*entry)->getval(magkey);
    
    if(flux<=0) continue;
    
    double contamination_of_neighbour = (*entry)->getval("neic");
    
    if(contamination_of_neighbour/flux>contamination_cut) // cause contamination of neighbour
      continue;
    
    values[count++]=2.5*log10(flux)+mag;
  }
  if (count==0) {
    cerr << "no valid entries in catalog!" << endl;
    delete [] values;
    return EXIT_FAILURE;
  }
  double zp,rms;
  FILE *file = fopen(outputfilename.c_str(),"w");
  
  // gaussian
  zp = gaussianfit(values,count,zp,rms,3.,true);
  cout << zp << " " << rms << endl;
  if(rms>0) { 
    zp = gaussianfit(values,count,zp,rms,2.,false);
    cout << zp << " " << rms << endl;
    zp = gaussianfit(values,count,zp,rms,1.5,false);
    cout << zp << " " << rms << endl;
    zp = gaussianfit(values,count,zp,rms,1.,false);
    cout << zp << " " << rms << endl;
  }else{
    rms=-rms;
  }
  fprintf(file,"@PSFZP %6.6f\n",zp);
  fprintf(file,"@PSFZPERROR %6.6f\n",rms);
  fprintf(file,"@NMEASUREMENTS %d\n",count);
  
  printf("@PSFZP %6.6f %6.6f %d\n",zp,rms,count);
  if(nstars>0) {
    fprintf(file,"@NSTARS %d\n",nstars);
  }
  if(nimages>0) {
    fprintf(file,"@NIMAGES %d\n",nimages);
  }
  
  delete [] values;
  
  
  // build a processed star list
  
  
  ofstream stream("zpstars.list");
  stream << "# ira : as in calib. catalog" << endl;
  stream << "# idec : as in calib. catalog" << endl;
  stream << "# fra : fitted ra" << endl;
  stream << "# fdec : fitted dec" << endl;
  stream << "# ix : input x" << endl;
  stream << "# iy : input y" << endl;
  stream << "# fx : fitted x" << endl;
  stream << "# fy : fitted y" << endl;
  stream << "# id : number in calib. catalog" << endl;
  stream << "# mag : mag derived from match to catalog." << endl;
  stream << "# error : error on mag." << endl;
  stream << "# flux : flux in ref. image units" << endl;
  stream << "# fluxrms : scatter around the above" << endl;
  stream << "# mneas : number of measurements involved in the average" << endl;
  stream << "# zpchi2 : chi2pdf ass. stable star without sys. effect" << endl;
  stream << "# zpchi2_01 : chi2pdf ass. stable star with 0.01 sys. effect" << endl;
  stream << "# nmag : level of contamination from neighbours (mag.)" << endl;
  stream << "# ndist : distance of nearest neighbour (=neid, pixels)" << endl;
  stream << "# mag_min : min. meas. magnitude" << endl;
  stream << "# mag_max : max. meas. magnitude" << endl;
  stream << "# sky : av. fitted residual sky level" << endl;
  stream << "# psfchi2 : chi2 pdf of PSF photometry" << endl;
  stream << "# satur : 0=ok" << endl;
  stream << "# contam : 0=ok, else contamination" << endl;
  stream << "# var : 0=ok else variable star" << endl;
  stream << "# nostar : 0=ok else probably a galaxy" << endl;   
  stream << "# g : " << endl;
  stream << "# ge : " << endl;
  stream << "# r : " << endl;
  stream << "# re : " << endl;
  stream << "# i : " << endl;
  stream << "# ie : " << endl;
  stream << "# z : " << endl;
  stream << "# ze : " << endl;
  stream << "# end " << endl;
  
  zpstar current_star;
  current_star.zp = zp;
  
  double sum_chi2_of_images = 0; // compute total chi2 with zp and star magnitudes as free parameters
  double sum_ndf_of_images  = 0; // 
  double sum_chi2_of_images_01 = 0; // same, assuming 0.01 sys. effect on, say, flatfielding or psf
  
  double sum_chi2_of_stars = 0; // compute total chi2 once dispersion between measurements of a star propagated into error
  double sum_ndf_of_stars  = 0; 
  
  for(DicStarCIterator entry=catalog.begin();entry!=catalog.end();++entry) {
    
    // if new star, dump results here
    if(current_star.id != (*entry)->getval("star")) {
      current_star.process();
      if(current_star.id>0) {
	stream << current_star << endl;
	sum_chi2_of_images += current_star.zpchi2*current_star.nmeas();
	sum_chi2_of_images_01 += current_star.zpchi2_01*current_star.nmeas();
	sum_ndf_of_images += current_star.nmeas() - 1; // -1 for the magnitude free parameter	
	sum_chi2_of_stars += sqr(current_star.mag-current_star.catalog_mag)/(sqr(current_star.catalog_mage)+sqr(current_star.error));
	sum_ndf_of_stars +=1;
      }
      current_star.reset();
    }
    current_star.load(*entry);            
  }
  stream.close();
  
  sum_ndf_of_images -= 1 ; // for the free zp parameter
  sum_ndf_of_stars -= 1 ; // for the free zp parameter
  
  fprintf(file,"@CHI2PDF_OF_IMAGES %6.6f\n",sum_chi2_of_images/sum_ndf_of_images);
  fprintf(file,"@CHI2PDF_OF_IMAGES_01 %6.6f\n",sum_chi2_of_images_01/sum_ndf_of_images);
  fprintf(file,"@NDF_OF_IMAGES %6.6f\n",sum_ndf_of_images);   
  fprintf(file,"@CHI2PDF_OF_STARS %6.6f\n",sum_chi2_of_stars/sum_ndf_of_stars);
  fprintf(file,"@NDF_OF_STARS %6.6f\n",sum_ndf_of_stars);  
  fclose(file);
  return EXIT_SUCCESS;
}


