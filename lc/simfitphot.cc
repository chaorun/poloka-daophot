#include <algorithm> // min_element

#include <fastfinder.h>
#include <reducedutils.h>

#include "simfit.h"
#include "simfitphot.h"

#define FNAME
#define DEBUG

static bool IncSeeing(const CountedRef<ReducedImage> one, const CountedRef<ReducedImage> two)
{ return (one->Seeing() < two->Seeing());}


static void init_phot(LightCurveList& Fiducials)
{  
#ifdef FNAME
  cout << " > init_phot(LightCurveList& Fiducials)" << endl;
#endif
  if (Fiducials.size() == 0) 
    {
      cerr << " init_phot() : no objects to initialize! \n";
      return;
    }

  // if no given reference, make it as best seeing image
  if (!Fiducials.RefImage) 
    {
      
      Fiducials.RefImage = *min_element(Fiducials.Images.begin(), Fiducials.Images.end(), IncSeeing);

      for_each(Fiducials.Objects.begin(), Fiducials.Objects.end(),
	       bind2nd(mem_fun(&Fiducial<PhotStar>::AssignImage), Fiducials.RefImage));
    }
 
  cout << " init_phot() : initalizing photometry for " << Fiducials.size() << " objects " << endl;

  const double maxDist = 2.;
  const SEStarList sexRefCat(Fiducials.RefImage->CatalogName());
  const BaseStarList *refCat(SE2Base(&sexRefCat));

  for (ReducedImageCIterator it=Fiducials.Images.begin(); it != Fiducials.Images.end(); ++it)
    {
      const ReducedImage *itRed  = *it;
      const SEStarList sexCat(itRed->CatalogName());
      const BaseStarList *cat(SE2Base(&sexCat));
      const FastFinder sexFinder(*cat);
      const Frame thisFrame = itRed->UsablePart();
      const double ratio = MedianPhotomRatio(*cat, *refCat);

      cout << " " << itRed->Name() << " " << Fiducials.RefImage->Name() 
	   << " median ratio = " << ratio << endl;

      for (LightCurveList::iterator si=Fiducials.begin(); si != Fiducials.end(); ++si)
	{
	  if (!thisFrame.InFrame(*si->Ref))
	    {
	      cerr << " init_phot() : Warning : " 
		   << si->Ref->x << " " << si->Ref->y << " is outside of image " << itRed->Name() << endl;
	      continue;
	    }

	  PhotStar *fidPhot = new PhotStar(BaseStar(si->Ref->x, si->Ref->y, 0.));
	  const SEStar *fidSex = (SEStar *) sexFinder.FindClosest(*si->Ref, maxDist);
	  if (fidSex) 
	    { 
	      delete fidPhot;
	      fidPhot = new PhotStar(*fidSex);
	      fidPhot->flux    /= ratio;
	      fidPhot->varflux /= ratio*ratio;
	    }
	  si->push_back(*it, fidPhot);
	}
    }
}

SimFitPhot::SimFitPhot(LightCurveList& Fiducials)
{
#ifdef FNAME
  cout << " > SimFitPhot::SimFitPhot(LightCurveList& Fiducials)" << endl;
#endif
  init_phot(Fiducials);
#ifdef DEBUG
  cout << "  zeFit.reserve ... " << endl;
#endif
  zeFit.reserve(Fiducials.Images.size());
  
  // 4 fwhm maximum size vignets
  const double rad = 4.*2.3548;

  zeFit.VignetRef = SimFitRefVignet(Fiducials.RefImage, int(ceil(Fiducials.RefImage->Seeing()*rad)));
  
  double worstSeeing = -1e29;
  
  for (ReducedImageCIterator it=Fiducials.Images.begin(); it != Fiducials.Images.end(); ++it)
    {
      const double curSeeing = (*it)->Seeing();
      SimFitVignet *vig = new SimFitVignet(*it);
      zeFit.push_back(vig);
      if (worstSeeing < curSeeing) worstSeeing = curSeeing;
    }
 
  zeFit.FindMinimumScale(worstSeeing);
}


void SimFitPhot::operator() (LightCurve& Lc)
{
  zeFit.Load(Lc);

  switch (Lc.Ref->type)
    {
    case 0: zeFit.SetWhatToFit(FitFlux | FitPos | FitGal); break; // sn+galaxy
    case 1: zeFit.SetWhatToFit(FitFlux | FitPos         ); break; // star
    case 2: zeFit.SetWhatToFit(                   FitGal); break; // galaxy
    default: cerr << " SimFitPhot::operator() : Error : unknown star type :" 
		  << Lc.Ref->type << endl; return;
    }

  zeFit.DoTheFit();
}

