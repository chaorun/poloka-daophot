// This may look like C code, but it is really -*- C++ -*-



#ifndef SESTAR_SEEN
#define SESTAR_SEEN

#include <iostream>

#include "persistence.h"

#include "basestar.h"
#include "image.h"

using namespace std;


//********************   DEFINITION SEStar   *********************

// pixel de coordonnees (i=0, j=0) a pour coordonnees
// pour nous (x=0., y=0.) et pour SExtractor (xse = 1, yse=1)
// see analyse.c


// si on ajoute -1 au cat SExtractor, alors SE==IJ
#define DECALAGE_CAT_SE -1.0

#define DECALAGE_SE_IJ 0.
#define DECALAGE_IJ_SE 0.

#define DECALAGE_SE_XY (DECALAGE_SE_IJ + DECALAGE_IJ_XY ) // 0. 
#define DECALAGE_XY_SE (DECALAGE_XY_IJ + DECALAGE_IJ_SE ) // 0. 


/*! \file */
 
//! SExtractor star
/*! The flux of BaseStar is   FLUX_BEST from SExtractor, i.e.
  FLUX_ISOCOR if not crowded,  FLUX_AUTO otherwise. */ 
class SEStar : public BaseStar {
  CLASS_VERSION(SEStar,1);
  #define SEStar__is__persistent
public:
  SEStar();
  SEStar(double xx, double yy, double ff);



  /*DOC \noindent{\bf Parameters and their meaning:}  \\
  X, Y, Flux de BaseStar = \\
  X_IMAGE, Object position along x \\
  Y_IMAGE, Object position along y \\
  FLUX_BEST= FLUX_ISOCOR if not crowded,  FLUX_AUTO otherwise */ 
  //!
  bool HasNeighbours() const  ;
  //!
  bool IsBlended() const;
  //!
  bool IsBad() const;
  //!
  bool IsCosmic() const;
  //!
  bool IsSaturated(const double saturation) const;
  //!
  bool IsSaturated() const;
  //!
  bool IsTruncated() const;
  //!
  void FlagAsSaturated();
  //!
  void FlagAsCosmic();
  //!
  void FlagAsNotSaturated();

  //!  x-coordinate of the brightest pixel 
    double X_Peak() const {return xpeak;}

  //!  y-coordinate of the brightest pixel 
    double Y_Peak() const {return ypeak;}

  //! RMS error for BEST flux 
  double EFlux() const {return e_flux;}


  //! Peak flux **above background**
  double Fluxmax() const {return fluxmax;}
 
  //! background
  double Fond()const {return fond;}

  //! SExtractor FLUX_AUTO: Flux within a Kron-like elliptical aperture 
  double Flux_aper() const {return flux_aper;}

  //! SExtractor FLUXERR_AUTO :  RMS error for Flux_aper
 double Eflux_aper() const {return e_flux_aper;} 
 
  /* SExtractor FLUX_APER: Flux within a Fixed aperture.
     Not Filled by SExtractor. Can be filled later using a
  routine from photometrie package */
  
  double Flux_fixaper() const {return flux_fixaper;}

  /* DOCF SExtractor FLUXERR_APER :  RMS error for Fluxfix_aper
     Not Filled by SExtractor */
 double Eflux_fixaper() const {return e_flux_fixaper;} 
 
  //! SExtractor FLUX_ISO: isophotal flux  
  double Flux_iso() const {return flux_iso;}
  
   //!  SExtractor FLUXERR_ISO: error on  isophotal flux 
 double Eflux_iso() const {return e_flux_iso;} 

  //!  SExtractor FLUX_ISOCOR: isophotal corrected flux 
  double Flux_isocor() const {return flux_isocor;}
  
   //!  SExtractor FLUXERR_ISOCOR error on  isophotal corrected flux 
 double Eflux_isocor() const {return e_flux_isocor;} 
 
  //! fwhm in pixels 
 double  Fwhm() const  {return fwhm;} 
 
  //! extension of the  aperture , in units of A or B
  double Kronradius() const {return kronradius;} 
  
  //! area of lowest isophote en pixels 
  double Isoarea() const {return isoarea;} 
 
   //!  <$x^2$ - <x>${}^2$>
  double Mxx() const {return mxx;}
   //!  <$y^2$ - <y>${}^2$>
  double Myy() const {return myy;}
  //!  <xy - <x><y>>
 double Mxy() const {return mxy;}



  //! Profile RMS along major axis 
  double A() const  {return  a;}
  
  //!  Profile RMS along minor axis
  double B() const {return  b;}
  
  //! gyration angle of the major axis 0.0 = axis en degres  Position angle
  double Gyr_Angle() const  {return  gyr_angle;} 
 

   //! Extraction flags 
 int Flag() const  {return flag;}
 
  //! number of bad pixels in iso area 
  int FlagBad() const  {return flagbad;}
  
   //!  star/galaxy classification: [0=gal ... 1=star] 
  double  Cstar() const  {return cstar;}
  
  //!  local barycenter (see Pierre) 
  double  Xtrunc() const  {return xtrunc;}
  //!  local barycenter 
  double  Ytrunc() const  {return ytrunc;}

  // star number (easier to keep track, and needed by daophot)
  int N() const  {return num;}

  //! Number of iteration in the PSF fitting (ALLSTAR) procedure
  int Iter() const {return iter;} 

  //! chi from fit (to be plot vs. magnitude)
  double Chi() const {return chi;}

  //! sharp diagnostic (to be plot vs. magnitude)
  double Sharp() const {return sharp;}
  

  /*DOC All these functions are also defined as double& / int&.\\ */

  double& X_Peak()  {return xpeak;}
  double& Y_Peak()  {return ypeak;}
  double& EFlux()  {return e_flux;}
  double& Fluxmax()  {return fluxmax;}
  double& Fond()  {return fond;}
  double& Flux_aper()  {return flux_aper;}
  double& Eflux_aper()  {return e_flux_aper;} 
  double& Flux_fixaper()  {return flux_fixaper;}
  double& Eflux_fixaper()  {return e_flux_fixaper;} 
 
  double& Flux_iso()  {return flux_iso;}
  double& Eflux_iso()  {return e_flux_iso;} 
 
  double& Flux_isocor()  {return flux_isocor;}
  double& Eflux_isocor()  {return e_flux_isocor;} 
 
  double&  Fwhm()   {return fwhm;} 
 
  double& Kronradius()  {return kronradius;} 
  double& Isoarea()  {return isoarea;}
  double& Mxx() {return mxx;}
  double& Myy() {return myy;}
  double& Mxy() {return mxy;}
  double& A()   {return  a;}
  double& B()  {return  b;}
  double& Gyr_Angle()   {return  gyr_angle;} 


  int& Flag()   {return flag;}
  int& FlagBad()   {return flagbad;}
  double&  Cstar()   {return cstar;}
  double&  Xtrunc()  {return xtrunc;}
  double&  Ytrunc()  {return ytrunc;}
  int& N(){return num;}
  int& Iter() {return iter;}
  double& Chi() {return chi;}
  double& Sharp() {return sharp;}


  /* DOC \noindent {\bf For read & write}: */
  
  //! for dump with NO end-of-line
  virtual void    dumpn(ostream& s = cout) const;

  //! for dump
  virtual void    dump(ostream& s = cout) const ;
  
  //! for write with NO end-of-line
  virtual void    writen(ostream& s = cout) const ;

  //! to read once the object is created 
  virtual void    read_it(istream& r, const char *Format); 

   //! to read and create the object  

  static SEStar* read(istream& r, const char* Format); 

  /* DOCF  to write the StarList header with the string ${}^{\star}i$
     appended to every ntuple variable (with no end)  */
  std::string WriteHeader_(ostream & pr = cout, const char* i = NULL) const;

 
 
  static const char *TypeName() { return "SEStar";}



  // n'est pas saturee  ni cosmique/colonne

    bool IsOK(const double &saturation) const;


private:
  // uniquement appelee par le constructeur, donc private  
void Set_to_Zero();

protected:
  int num ;
  double xpeak;
  double ypeak ;
  double fluxmax ; 
  double e_flux  ; 
  double fond ;
  double flux_aper  ;
  double e_flux_aper  ;
  double flux_fixaper  ;
  double e_flux_fixaper  ;
   double flux_iso ; 
  double e_flux_iso ;
  double flux_isocor ; 
  double e_flux_isocor ;
  double kronradius ;
  double isoarea ;
  double fwhm ; 
  double mxx ;
  double myy ;
  double mxy ;
  double a ; 
  double b ; 
  double gyr_angle ;
  int flag; 
  int flagbad; 
  double cstar ;  
  double xtrunc ;  
  double ytrunc ;  
  int iter;
  double chi;
  double sharp;

#ifndef SWIG
  ClassDef(SEStar,1) // Root's stuff.
#endif /* SWIG */

};



//! sort in decreasing peak value above background 
bool 
DecFluxMax(const SEStar *S1, const SEStar *S2);




/********************   FIN DEFINITION SEStar   **********************/
/* what concerns the SEStarList's : */
#include <list>
#include "starlist.h"

//@{
//! definition of the list and iterators
#ifdef USE_ROOT
typedef StarListWithRoot<SEStar> SEStarList;
#else
typedef StarList<SEStar> SEStarList;
#endif /* USE_ROOT */

typedef SEStarList::const_iterator SEStarCIterator;
typedef SEStarList::iterator SEStarIterator;
typedef CountedRef<SEStar> SEStarRef;
//@}

#ifndef SWIG
//! type casting
BaseStarList* SE2Base(SEStarList * This);
const BaseStarList* SE2Base(const SEStarList * This);
#endif


int 
FlagCosmicFromImage(SEStarList & stl, 
		    Image const & image, const double dist);
int
FlagSaturatedStars(SEStarList & stl, const double saturation);

int 
FlagSatFromImage(SEStarList & stl, Image const & image);


//! Select stars with the IsOK(saturation) routine.
int KeepIt(double saturation ,SEStarList const & stli,SEStarList & temp);

//! Do as indicated in name ........
void 
SetStarsBackground(SEStarList & stl, double const background);

//! Select Stars in SEStarList
int StarFinder(SEStarList & stlse, SEStarList & PrettyStars, double saturation=1.e+30);

#endif //SESTAR_SEEN


