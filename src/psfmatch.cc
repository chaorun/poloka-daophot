#include <iostream>
#include <fstream>

#include "psfmatch.h"
#include "fitsimage.h"
#include "kernelfit.h"
#include "sestar.h"
#include "fastfinder.h"
#include "reducedutils.h"


#define DEBUG_PsfMatch

static bool DecreasingFluxMax(const SEStar *S1, const SEStar *S2)
{
  return (S1->Fluxmax() > S2->Fluxmax());
}

//both images are supposedly already registered (i.e. aligned)
PsfMatch::PsfMatch(const ReducedImageRef Ref, const ReducedImageRef New, const PsfMatch *APreviousMatch, bool noswap)
{
   refName = Ref->Name();
  newName = New->Name();
  ref_is_best = true; // by default
  if (APreviousMatch) 
    { 
      fit = APreviousMatch->fit;
      if (fit == NULL ) cerr <<"ERREUR in PSFMatch, null pointeur. " << endl ;
    }
  if(noswap) 
    {
      best = Ref;
      worst = New;
      cout << " PsfMatch between " << Ref->Name()
	   << "(ref=best) and " << New->Name() << " (new)" << endl; 
    }
  else 
    {

      ref_is_best = (Ref->Seeing() <= New->Seeing());
      if (ref_is_best)
	{
	  best = Ref;
	  worst = New;
	}
      else
	{
	  best = New;
	  worst = Ref;
	}
      cout << " PsfMatch between " << Ref->Name() << "(ref) and " << New->Name() << " (new) : " 
	   << best->Name()  << " is the best one" << endl;
    }
  

  photomRatio=1;
  seeing = worst->Seeing();
  sigmaBack = 0;
  intersection = Ref->UsablePart() * New->UsablePart(); 

  if (APreviousMatch) {
    photomRatio= APreviousMatch->PhotomRatio() ;
    sigmaBack = APreviousMatch->SigmaBack() ;
  }
}

#ifdef STORAGE // specialement moche
PsfMatch::PsfMatch(const PsfMatch &Original)
{
  memcpy(this, &Original, sizeof(Original));
  //  this->fit = new KernelFit(*Original.fit); just does not work because KernelFit's default copy cannot work;
  shouldNotDeleteFit = true;
}
#endif

PsfMatch::PsfMatch() // needed by any IO system
{
  //fit = NULL; initialise par le constructeur de CountedRef
  //best = NULL; idem
  //worst = NULL;idem
}

PsfMatch::~PsfMatch()
{
  if (fit) 
    {
      // really a good idea ? (P.A)
      // yes because in most cases PsfMatch allocated them.
      if (fit->WorstImage) delete (fit->WorstImage);
      if (fit->BestImage) delete (fit->BestImage);
      //delete fit; + necessaire avec KernelFitRef
    }
}

// another starlist selection routine
static bool GoodForFit(const SEStar *Star, const double &SaturLev, const double &Bmin, const double& minsignaltonoiseratio)
{
  return ((Star->FlagBad() == 0 ) && 
	  ( Star->Flag() < 3 )   && //(keep blended objects) 
	  ( Star->Fluxmax()+Star->Fond() < SaturLev ) &&
	  ( Star->B() > Bmin ) &&
	  ( Star->Flux_aper()/Star->Eflux_aper() > minsignaltonoiseratio)
	  );
}

static bool check_weights(const Image& weight, const double& x, const double& y,  const int &hsize) {
  
  int xmin = int(x)-hsize; if(xmin<0) return false;
  int xmax = int(x)+1+hsize; if(xmax>=weight.Nx()) return false;
  int ymin = int(y)-hsize; if(ymin<0) return false;
  int ymax = int(y)+1+hsize; if(ymax>weight.Ny()) return false;
  
  
  for(int j=ymin; j<ymax; j++)
    for(int i=xmin; i<xmax; i++)
      if( weight(i,j)==0) return false;
  return true;
}

string PsfMatch::NotFilteredStarListName()
{
  string ListName = "/kept.list";
  if (FileExists(best->Name()+ListName))
    return best->Name()+ListName;
  if(FileExists(worst->Name()+ListName))
    return worst->Name()+ListName;
  else
    return "";
}

int PsfMatch::FilterStarList(const double MaxDist)
{
  //get list of objects suitable for the kernel fit 
  SEStarList bestStarList(best->ImageCatalogName());

  cout << " Entering FilterStarList with "<< bestStarList.size() 
       << " stars " << endl;
  //star list selection
  SEStarList worstStarList(worst->ImageCatalogName());
  double satfactor = 0.95;
  double saturLevBest = best->Saturation() * satfactor;
  double saturLevWorst = worst->Saturation() * satfactor;  
  double bfactor = 0.2;
  double bMinBest = best->Seeing() * bfactor;
  double bMinWorst = worst->Seeing()* bfactor;
  double mindist = worst->Seeing()*3;
  double minsignaltonoiseratio = 10;
  cout << "  cuts satur bmin " << saturLevBest << " " << bMinBest << endl;
  FastFinder worstFinder(*SE2Base(&worstStarList));

 
  
  for (SEStarIterator sibest = bestStarList.begin(); sibest != bestStarList.end(); )
    {
      SEStar *sb = *sibest;
      const SEStar *sworst;  
      if (GoodForFit(sb, saturLevBest, bMinBest, minsignaltonoiseratio)
	  && (sworst = (SEStar *) worstFinder.FindClosest(*sb,MaxDist))
	  && (sb->Distance(*sworst) < MaxDist) // useless?
	  && GoodForFit(sworst, saturLevWorst, bMinWorst, minsignaltonoiseratio)
	  && (intersection.InFrame(*sb))  
	  && (intersection.MinDistToEdges(*sb) > mindist)) // remove objects close to the edge
	{
	  ++sibest;
	}
      else
	{
	  sibest = bestStarList.erase(sibest);
	}
    }
  // get size of stamps to check for null weights for all selected objects
  OptParams optparams;
  optparams.OptimizeSizes(best->Seeing(),worst->Seeing());
  // open weights
  FitsImage weight(best->FitsWeightName());
  {
    FitsImage satur(best->FitsSaturName());
    weight *= (1-satur);
  }
  {
    FitsImage worst_weight(worst->FitsWeightName());
    weight *= worst_weight;
  }
  {
    FitsImage satur(worst->FitsSaturName());
    weight *= (1-satur);
  }
  int hsize = optparams.HKernelSize+optparams.HStampSize;
  
  for (SEStarIterator sibest = bestStarList.begin(); sibest != bestStarList.end(); )
    {
      SEStar *sb = *sibest;
      if( check_weights(weight,sb->x,sb->y,hsize)) 
	++sibest;
      else
	sibest = bestStarList.erase(sibest);
    } 
  
  if (getenv("DUMP_FIT_LIST"))
    bestStarList.write("fitting_objects.list");
  bestStarList.sort(DecreasingFluxMax);
  SE2Base(&bestStarList)->CopyTo(objectsUsedToFit);
  return objectsUsedToFit.size();
}

static double sqr(const double &x) { return x*x;}

static KernelFitRef 
init_and_do_the_fit(const BaseStarList& objectsUsedToFit, 
		    const ReducedImage *best,
		    const ReducedImage *worst,
		    const Image* BestImage = NULL,
		    const Image* WorstImage = NULL)
{
  KernelFitRef fit = new KernelFit;

  cout << " Entering KernelFit between " << best->Name() << " and " << worst->Name() << endl;

  fit->BestImageBack = best->BackLevel();
  fit->WorstImageBack = worst->BackLevel();

  //access to data
  fit->WorstImage = (WorstImage) ? WorstImage : new FitsImage(worst->FitsName());
  fit->BestImage =  (BestImage)  ? BestImage : new FitsImage(best->FitsName());

  // Variance for the Chi2
  Pixel mean,sigma;

  FitsHeader hworst(worst->FitsName());
  FitsHeader hbest(best->FitsName());
  Frame dataRegionW(hworst);
  Frame dataRegionB(hbest);
  fit->DataFrame = dataRegionW*dataRegionB;
  fit->WorstImage->SkyLevel(dataRegionW,&mean,&sigma);
  fit->SkyVarianceWorstImage = sigma*sigma;
  fit->WorstImageGain = worst->Gain();
  if (fit->WorstImageGain == 0)
    {
      cerr << " Error: PsfMatch: cannot find a valid gain in image " 
	   << worst->Name() << ", set it to 1." << endl;
      fit->WorstImageGain = 1;
    }

  //fit
  double badseeing = worst->Seeing();
  double goodseeing = best->Seeing();
  if (!(fit->DoTheFit(objectsUsedToFit, goodseeing, badseeing)))
    {
      if (BestImage) delete fit->BestImage;
      if (WorstImage) delete fit->WorstImage;
      //delete fit; fit = NULL;
      fit = KernelFitRef() ;
    }
  return fit;
}



bool PsfMatch::FitKernel(const bool KeepImages)
{
  unsigned nstars = objectsUsedToFit.size();
  if (NotFilteredStarListName() == "")
    {
      if (nstars==0) nstars = FilterStarList();
      cout << " We have " << nstars << " objects to fit with" << endl;
      if (nstars < 10) cerr << " WARNING: Less than 10 common objects for kernelfit "<< endl;
    }
  else
    {
      BaseStarList bsl(NotFilteredStarListName());
      bsl.CopyTo(objectsUsedToFit);
    }

  //objectsUsedToFit.sort(DecreasingFluxMax); // done in FilterStarList
  
  if(getenv("DUMP_FIT_LIST"))
    objectsUsedToFit.write(best->Name()+worst->Name()+".fit.list");
  cout << " Frame limits for the fit: " << intersection;

  KernelFitRef direct_fit = init_and_do_the_fit(objectsUsedToFit,best,worst);

  // check if fit exists
  if(direct_fit==NULL) {
    cout << "PsfMatch_SUMMARY_best_worst_kernelphotomratio_sextractorratio_nstamps_chi2/dof " 
	 << best->Name() << " "
	 << worst->Name() << " "
	 << -1 << " " // photomRatio
	 << 0 << " "  // sexRatio
	 << 0 << " "  // nstamps
	 << -1 << " " // chi2
	 << endl;
    return false;
  }


  fit = direct_fit; // le precedent fit est delete au decrochage
  
  // HERE IS THE CUT FOR THE SWAP OF IMAGES WHEN SEEINGS ARE CLOSE BY
  // I put the cut at 4 which is 3 sigmas from the mean of chi2 distribution
  // for all d3 subtractions from mars 2003 to june 2003
  // it represents 3% of the cases. As it is just a question of CPU
  // we can live with that

  //  if (!direct_fit || direct_fit->chi2 > 4.)
  if (false)
    {
      cout << " bad_direct_fit : trying swapped one (best<->worst)" << endl;
      KernelFitRef reversed_fit = 
	init_and_do_the_fit(objectsUsedToFit,worst, best, 
			    direct_fit ? direct_fit->WorstImage: NULL,
			    direct_fit ? direct_fit->BestImage : NULL);

      if (reversed_fit->chi2 < direct_fit->chi2)
	{
	  cout << " keeping swapped fit" << endl;
	  fit = reversed_fit;
	  //delete direct_fit;
	  direct_fit = KernelFitRef() ; // ie equiv. to direct_fit = NULL.
	  swap(worst,best);
	  ref_is_best = !ref_is_best;
	}
      else
	{
	  cout << " forget the swapped fit, keeping direct one " << endl;
	  //delete reversed_fit;
	  reversed_fit = KernelFitRef()  ;
	}
    }

  //update parameters
  seeing = max(worst->Seeing(), best->Seeing());  
  photomRatio = fit->KernAtCenterSum;
  if (!ref_is_best) photomRatio = 1.0/photomRatio;
  sigmaBack = sqrt(sqr(best->SigmaBack()*photomRatio) + sqr(worst->SigmaBack()));
  
  // compute sextractor photom-ratio to check everything was ok
  double sexRatio = 0;
  {
    const SEStarList best_sexCat(best->CatalogName());
    const SEStarList worst_sexCat(worst->CatalogName());
    const BaseStarList *best_cat = SE2Base(&best_sexCat);
    const BaseStarList *worst_cat = SE2Base(&worst_sexCat);
    sexRatio = MedianPhotomRatio(*worst_cat,*best_cat);
  }
  cout << "PsfMatch_SUMMARY_best_worst_kernelphotomratio_sextractorratio_nstamps_chi2/dof " 
       << best->Name() << " "
       << worst->Name() << " "
       << photomRatio << " "
       << sexRatio << " "
       << fit->NStampsUsed() << " "
       << fit->chi2 << " "
       << endl;
  
  //cout << " Final seeing = " << seeing << endl;
  //cout << " Photometric ratio (New/Ref) = " << photomRatio << endl;
  //cout << " Combined images predicted sigma = " << sigmaBack << endl;

  // dump some info about the fitted kernel
  int stepx = fit->BestImage->Nx()/4;
  int stepy = stepx;
  Kernel kern(fit->optParams.HKernelSize);
  for (int i= stepx/2; i <fit->BestImage->Nx(); i+= stepx)
    for (int j = stepy/2; j < fit->BestImage->Ny(); j+=stepy)
      {
	fit->KernCompute(kern, double(i), double(j));
	cout << " kernel caracteristics at " << i << ' ' << j; kern.dump_info();
      }
	
  // free image memory if requested;
  if (!KeepImages)
    {
      if (fit->WorstImage) delete fit->WorstImage; fit->WorstImage = NULL;
      if (fit->BestImage) delete fit->BestImage ; fit->BestImage =  NULL;  
    }
  return true;
}

double PsfMatch::Chi2() const {return fit->chi2;}
int PsfMatch::Nstars() const {return fit->NStampsUsed();}
int PsfMatch::Nparams() const {return fit->mSize;}


#include "quali_box.h"

// this routine creates a kind of ntuple that allows to study 
// the subtraction quality  (the famous Delphine's plot)
static void  quali_plots(const ReducedImage *Ref, const ReducedImage *New, 
	    const FitsImage *subImage, const string LogName)
{
  const Image *imArray[3];
  FitsImage refImage(Ref->FitsName());
  FitsImage newImage(New->FitsName());
  imArray[0] = &refImage;
  imArray[1] = &newImage;
  imArray[2] = subImage;
  double fond[3] = {Ref->BackLevel(), New->BackLevel(), 0};  /* the last one is zero because the kernelfit is supposed to achieve that ... */
  double sigmaFond[3] = {Ref->SigmaBack(), New->SigmaBack(),
			 sqrt(sqr(Ref->SigmaBack())+sqr(New->SigmaBack()))} ;
  /* bon d'accord, le dernier est ``faux'' mais chez Delphine aussi. 
     alors comme c'est pour faire des comparaisons... */

  SEStarList refList(Ref->CatalogName());
  SEStarList newList(New->CatalogName());
  ofstream qual_log(LogName.c_str());
  GtransfoIdentity ident;

  Test_Qual_N_same(3, *subImage, refList, newList,
		   &ident, imArray, fond, sigmaFond, 10, qual_log);
  qual_log.close();
}                 



//! builds the subtraction image new-ref (in photometric units of the ref). Puts it as the fits image of RImage. Sets seeing & co. Does not keep by default the result of the convolution. 
#include "fileutils.h" // for MakeRelativeLink

bool PsfMatch::VarianceConvolve(Image &Variance)
{
  if (!fit) 
    {
      cout << " ERROR : PsfMatch::ConvolveVariance : cannot do that without a fit " << endl;
      return false;
    }
  // cannot convolve "in place : " 
  Image *result = fit->VarianceConvolve(Variance);
  // overwrite the input
  Variance = *result;
  // delete temporary
  delete result;
  return true;
}
    
void PsfMatch::ConvolveBest(ReducedImage &ConvImage)
{
  cout << " Entering ConvolveBest "<< best->FitsName() << " " 
       << ConvImage.FitsName() << " " << ConvImage.Name() << endl;
  if (!fit) FitKernel(true /* keep images in memory */);
  else fit->BestImage =  new FitsImage(best->FitsName());
  if (!fit->ConvolvedBest) fit->BestImageConvolve();
  FitsHeader hworst(worst->FitsName()); 
  FitsImage cImage(ConvImage.FitsName(), hworst,
		   Image(worst->XSize(),worst->YSize()));
  cImage.SetWriteAsFloat();
  cImage.AddOrModKey("KERNREF",best->Name().c_str(), " name of the seeing reference image");
  cImage.AddOrModKey("KERNCHI2",fit->chi2, " chi2 of the kernel fit");
  cImage.AddOrModKey("PHORATIO",photomRatio, " photometric ratio with KERNREF");

  string worstpsfname = worst->ImagePsfName();
  if (FileExists(worstpsfname)) 
    MakeRelativeLink(worstpsfname.c_str(), ConvImage.ImagePsfName().c_str());

#ifdef DEBUG_PsfMatch
  cout << "In convolvebest:  Update values " << endl;
#endif
  
  // Update values
  ConvImage.SetSeeing(seeing);
  ConvImage.SetBackLevel(photomRatio*best->BackLevel());
  ConvImage.SetSaturation(photomRatio*best->Saturation()," image saturation");
  ConvImage.SetExposure(photomRatio*best->Exposure());
  ConvImage.SetSigmaBack(photomRatio*best->SigmaBack());
  ConvImage.SetReadoutNoise(photomRatio*best->ReadoutNoise());
  ConvImage.SetFlatFieldNoise(photomRatio*best->FlatFieldNoise());
  ConvImage.SetOriginalSkyLevel(photomRatio*best->OriginalSkyLevel());
  ConvImage.SetOriginalSaturation(photomRatio*best->OriginalSaturation());

#ifdef DEBUG_PsfMatch
  cout << "In convolvebest:  weights: ConvImage.FitsWeightName = " << ConvImage.FitsWeightName() << endl;
  cout << "In convolvebest:  weights: best->FitsWeightName = " << best->FitsWeightName() << endl; 
#endif
  FitsHeader hbest(best->FitsWeightName());
  FitsImage weight(ConvImage.FitsWeightName(), hbest);
#ifdef DEBUG_PsfMatch
  cout << "In convolvebest: VarianceConvolve" << endl;
#endif
  VarianceConvolve(weight);
#ifdef DEBUG_PsfMatch
  cout << "In convolvebest: MakeCatalog" << endl;
#endif
  ConvImage.MakeCatalog();
#ifdef DEBUG_PsfMatch
  cout << " convolvebest done" << endl;
#endif
}

bool PsfMatch::Subtraction(ReducedImage &RImage, bool KeepConvolvedBest)
{
  if (!fit && !FitKernel(true /* keep images in memory */))
    {
      cerr << " cannot fit the kernel, giving up for subtraction " << endl;
      return false;
    }
  else
    {
      fit->WorstImage = new FitsImage(worst->FitsName());
      fit->BestImage =  new FitsImage(best->FitsName());
    }

  //Produce the image and header. Choose the Ref Header to get it's WCS.
  FitsHeader href(Ref()->FitsName());
  FitsImage subImage(RImage.FitsName(), href);
  subImage.ModKey("BITPIX",16); // 16 bits are enough
  Image &theSubtraction = subImage;
  if (!fit->ConvolvedBest) fit->BestImageConvolve();
  /* by convention the subtraction is expressed in units of the ref.
     - photomRatio is New/Ref
     - Important point : the same trick has to be applied to the variance computation. 
     - the zero point guess is the ref one.*/
  if (ref_is_best)
    {
      cout << " Subtracting " << worst->Name() << " - " << best->Name() << endl;
      double factor = 1./photomRatio; // "*" is far faster than "/"
      theSubtraction = *(fit->WorstImage);
      theSubtraction -= *(fit->ConvolvedBest);
      theSubtraction *=factor;
    }
  else /* new was concolved and ConvolvedBest natches Ref photometry  */
    {
      cout << " Subtracting " << best->Name() << " - " << worst->Name() << endl;
      theSubtraction = *(fit->ConvolvedBest);
      theSubtraction -= *(fit->WorstImage);
    }  
  // force 16 bits when we have 32 images (eg swarped)
  subImage.SetWriteAsFloat();
  subImage.AddOrModKey("KERNREF",best->Name().c_str(), " name of the seeing reference image");
  subImage.AddOrModKey("KERNCHI2",fit->chi2, " chi2 of the kernel fit");
  subImage.AddOrModKey("PHORATIO",photomRatio, " photometric ratio with KERNREF");
  string datenew = New()->Date();
  subImage.AddOrModKey("TOADDATE",datenew," Date in UTC of last obs");
  if (!KeepConvolvedBest) fit->DeleteConvolvedBest();
  fit->DeleteWorstDiffBkgrdSubtracted();

  intersection.WriteInHeader(subImage);
  string worstpsfname = worst->ImagePsfName();
  if (FileExists(worstpsfname)) MakeRelativeLink(worstpsfname.c_str(), RImage.ImagePsfName().c_str());

  //Update values
  RImage.SetSeeing(seeing);
  RImage.SetBackLevel(0.0); // by construction
  RImage.SetSaturation(worst->Saturation()," worst image saturation");
  RImage.SetExposure(best->Exposure() + worst->Exposure());
  RImage.SetSigmaBack(sigmaBack);
  double readnoise = sqrt(sqr(worst->ReadoutNoise())+sqr(photomRatio*best->ReadoutNoise()));
  RImage.SetReadoutNoise(readnoise);

  cerr << "Flat Field Noise on worst (" << worst->FitsName() << ") : " << worst->FlatFieldNoise() << endl ;  
  cerr << "Flat Field Noise on best (" << best->FitsName() << ") : " << best->FlatFieldNoise() << endl ;
  double flatnoise = sqrt(sqr(worst->FlatFieldNoise())+sqr(photomRatio*best->FlatFieldNoise()));
  cerr << "Setting FlatFieldNoise on sub: " << flatnoise << endl ;
  RImage.SetFlatFieldNoise(flatnoise);

  RImage.SetOriginalSkyLevel(worst->OriginalSkyLevel() 
			     + photomRatio*best->OriginalSkyLevel(),
			     " sum of sky levels of the subtraction terms");
  RImage.SetOriginalSaturation(worst->OriginalSaturation()
			     + photomRatio*best->OriginalSaturation(),
			       "sum of image original saturations");

  double zero_point = Ref()->AnyZeroPoint();
  cerr << "Zero Point as taken from reference stack:" 
       << zero_point << " propagated to sub. " << endl ;
  RImage.SetZZZeroP(zero_point, "as taken from reference stack");

  if (fit->WorstImage) delete fit->WorstImage; fit->WorstImage = NULL;
  if (fit->BestImage) delete fit->BestImage ; fit->BestImage =  NULL;
  quali_plots(Ref(), New(), &subImage, RImage.Dir()+"/qual.log");
  return true;
}


void PsfMatch::KernelToWorst(Kernel &Result, const double &x, const double &y) const
{
  if (!fit) {cerr << " PsfMatch::KernelToWorst : fit does not exist yet" << endl;}
  int HSizeX = fit->HKernelSizeX(); 
  int HSizeY = fit->HKernelSizeY();
  Result = Kernel(HSizeX, HSizeY);
  fit->KernCompute(Result, x, y); 
}

void PsfMatch::BackKernel(Kernel &Diffback,const double &xc,const double &yc)
{
  int ic = int(xc);
  int jc = int(yc);
  int hx = Diffback.HSizeX();
  int hy = Diffback.HSizeY();
  for (int i=-hx; i<=hx; ++i)
    for (int j=-hy; j<=hy; ++j) Diffback(i,j) = fit->BackValue(ic+i,jc+j);
}


void PsfMatch::SetKernelFit(KernelFit *kernel) {
  fit = kernel;
  fit->KernelsFill(); // refill kernels (not fully persistent)
}
