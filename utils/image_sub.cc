#include <iostream>
#include <vector>

#include "psfmatch.h"
#include "fitsimage.h"
#include "imagesubtraction.h"

// for io
#include "kernelfit_dict.h"
#include "objio.h"
#include "typemgr.h"


static void usage(const char *progName)
{
  cerr << "Usage: " << progName << " best worst  " << endl;
  cerr << "  match PSF of <DbImage(s)> with a reference image (assumed of best seeing) \n" 
       << "   Note:both images have to be on the same pixel grid ! \n";

  exit(-1);
}

int main(int nargs, char **args)
{
  // if nothing is given
  if (nargs < 3){usage(args[0]);}
  
  bool makesub = false;
  
  CountedRef<ReducedImage> refimage = new ReducedImage(args[1]);
  CountedRef<ReducedImage> newimage = new ReducedImage(args[2]);

  // write kernel in xml file
  string outputfilename = newimage->Dir()+"/kernel_from_"+refimage->Name()+".xml";
  cout << "image_sub : writing kernel in " << outputfilename << " ..." << endl;
  obj_output<xmlstream> oo(outputfilename);

  
  if(makesub) {
    ImageSubtraction sub("sub",refimage,newimage);
    sub.MakeFits();
    oo <<*(sub.GetKernelFit());
  }else{
    PsfMatch psfmatch(refimage,newimage,NULL,true);
    psfmatch.FitKernel(true);
    oo << *(psfmatch.GetKernelFit());
  }
  
  oo.close();
  cout << "the end" << endl;
  
  
  return 0;
}
