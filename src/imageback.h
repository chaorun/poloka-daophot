
#include <string>
#include "image.h"

/*! \file 

    This class deals with the computation of background average and
    fluctuations.  The computation is carried out by the constructor,
    using an algorithm borrowed from Sextractor
    (http://terapix.iap.fr/sextractor) : the average and sigma of
    pixels are computed over rectangles of size MeshStepX*MeshStepY,
    once using all pixels, and a second time using only pixels within
    2 sigmas of the average. Then a median filter (of half width 1) is
    applied to both maps. 
*/


//! Image Back class to compute an image of the background 
class ImageBack {

  public:
  /*! The constructor for a square Mesh (MeshStepX=MeshStepY). */
     ImageBack(Image const &SourceImage, int MeshStep, 
	       const Image* Weight=NULL);
  /*! The constructor for a rectangular Mesh (MeshStepX!=MeshStepY). */
     ImageBack(Image const &SourceImage, int MeshStepX, int MeshStepY,
	       const Image* Weight=NULL);

     int Nx() const { return nx;}
     int Ny() const { return ny;}

     /*! returns the average background for pixel (i,j) in the coordinates
        of the original image. */
     Pixel Value(const int i, const int j) const;

     /*!  same for Rms. */
     Pixel Rms(const int i, const int j) const;
     /*! returns the average background image  in the coordinates
        of the original image. */
     Image* BackgroundImage();

     /* DOCF access the (small) image containing the average background values. */
     const Image& BackValue() const { return backValue;}

     /* DOCF same for the sigma. */
     const Image& BackRms() const { return backRms;}
//     ~ImageBack() { ~backRms();};


     /* DOCF write the data to fits files. Give a name acceptable
     as a file name. */
     int Write(string FileName);

     /* DOCF reads the data from fits files (this is a constructor). Give the FileName you gave
     previously to Write */
     ImageBack(char *FileName, const Image &Source);
      
  private :
     int meshStepX; /* in number of pixels of the source image */
     int meshStepY; /* in number of pixels of the source image */
     const Image &sourceImage;
     int nx,ny;
     Image backValue;
     Image backRms;
     const Image *weight;

     void do_one_pad(const Frame &Pad, double &mean, double &sigma);
     void do_it();
};
     
