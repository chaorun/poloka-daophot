#include <iostream>

#include "fitsimage.h"
#include "dbimage.h"
#include "fileutils.h"
#include "usnoutils.h"

static void usage(const char *pgname)
{
  cout << pgname << ' ' << " <dbimage name .... >" << endl
       <<   " (or  -i <fitsImage> -l <sex cat>)" << endl
       << " [-n] just print, won't alter WCS" << endl
       << " [-o] to overwrite previous match" << endl
       << " [-c <datacards>]" << endl
       << " [-a <astrometric catalog>] (if applicable, superseeds value in datacards)" << endl;
}


int main(int argc, char **argv)
{
  list<string> dbImageList;
  if (argc < 2)
    {
      usage(argv[0]);  return -1;
    }

  char *fitsName = NULL;
  char *listName = NULL;
  bool overwrite = false;
  for (int i=1;  i<argc; ++i) // first loop on arguments to read datacards name, if any
    {
      char *arg = argv[i];
      if (arg[0] != '-') continue;
      if (arg[1] == 'c')
	{
	  i++;MatchPrefs.ReadCards(string(argv[i]));
	  break;
	}
    }
  for (int i=1;  i<argc; ++i)
    {
      char *arg = argv[i];
      if (arg[0] != '-') 
	{
	  dbImageList.push_back(string(arg));
	  continue;
	}
      switch (arg[1]) 
	{
	case 'i' : i++; fitsName = argv[i]; break;
	case 'l' : i++; listName = argv[i]; break; 
	case 'n' : MatchPrefs.writeWCS = false; break;
	case 'o' : overwrite = true; break;
	case 'c' : i++;break; // datacards were already read above
	case 'a' : i++; MatchPrefs.astromCatalogName = argv[i];break;
	default : 
	  std:: cout << " don't understand " << argv[i] << std::endl;
	  usage(argv[0]); exit(0);
	}
    }

  if (fitsName && listName)
    {
      FitsHeader head(fitsName);
      if (head.HasKey("DZEROUSN")  && !overwrite && MatchPrefs.writeWCS && !MatchPrefs.asciiWCS)
	{
	  cout << "Match already done! " << endl;
	  exit(EXIT_SUCCESS);
	}	
      if(UsnoProcess(fitsName,listName, NULL))
	return EXIT_SUCCESS;
      else
	return EXIT_FAILURE;
    }
  

  for (list<string>::iterator i=dbImageList.begin(); i!= dbImageList.end(); ++i)
    {
      string name = *i;
      DbImage dbimage(name);
      if (!dbimage.IsValid())
	{
	  cerr << " Be careful ! " << name << " must be an image name ! " << endl;
	  continue;
	}

      string catalogName = dbimage.ImageCatalogName(SExtractor);
      if (!FileExists(catalogName.c_str()))
	{
	  cerr << "The SExtractor Catalogue associated to image " << name << " doesn't exist !! " << endl;
	  continue;
	}  
      string fitsFileName = dbimage.FitsImageName(Calibrated);
      if (!FileExists(fitsFileName))
	{
	  cerr << "The  calibrated fits image " << name << " doesn't exist !! " << endl;
	  continue;
	}
      {
	FitsHeader head(fitsFileName);
	if (head.HasKey("DZEROUSN") && !overwrite && MatchPrefs.writeWCS && !MatchPrefs.asciiWCS)
	  {
	    cout << "Match already done for " << name << endl;
	    continue;
	  }	
      }
      if(!UsnoProcess(fitsFileName, catalogName, &dbimage))
	return EXIT_FAILURE;

    } /* end of loop on arguments */

  return EXIT_SUCCESS;
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

