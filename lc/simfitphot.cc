#include <algorithm> // min_element

#include <fastfinder.h>
#include <reducedutils.h>

#include "simfit.h"
#include "simfitphot.h"

//#define DEBUG

static void init_phot(LightCurveList& Fiducials)
{  

  if (Fiducials.size() == 0) 
    {
      cerr << " init_phot() : no objects to initialize! \n";
      return;
    }

  // if no given reference, make it as best seeing image
  //if (!Fiducials.RefImage) 
  if (false)
    {
      
      //Fiducials.RefImage = *min_element(Fiducials.Images.begin(), Fiducials.Images.end(), IncreasingSeeing);

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
#ifdef DEBUG
  cout << " SimFitPhot::SimFitPhot" << endl;
#endif
  init_phot(Fiducials);

  zeFit.reserve(Fiducials.Images.size());
  
  // 4 fwhm maximum size vignets
  const double rad = 4.*2.3548;

  PhotStar *star = new PhotStar();
  
  zeFit.VignetRef = SimFitRefVignet(star, Fiducials.RefImage, 
				    int(ceil(Fiducials.RefImage->Seeing()*rad)));
  
  double worstSeeing = -1e29;
  
  for (ReducedImageCIterator it=Fiducials.Images.begin(); it != Fiducials.Images.end(); ++it)
    {
      const double curSeeing = (*it)->Seeing();
      SimFitVignet *vig = new SimFitVignet(star, *it, Fiducials.RefImage, int(ceil(curSeeing*rad)));
      zeFit.push_back(vig);
      if (worstSeeing < curSeeing) worstSeeing = curSeeing;
    }
 
#ifdef DEBUG
  cout << " SimFitPhot::SimFitPhot zeFit.FindMinimumScale" << endl;
#endif
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

