/*! \file
\brief 
Performs arithmetic operations involving one or two images

\details
Use: imarith image1[ext] image2[ext] operation outimage  (2 images)

or

imarith image[ext] value operation outimage         (1 image)

Add, subtract, multiply, or divide the pixels in one image by another
image or a constant and write the result to a new output
image. Supported arithmetic operations are 'add', 'sub', 'mul', or 'div'
(only the first character need be supplied)

Examples: 

- imarith in1.fit in2.fit add out.fit     - add the 2 images
- imarith in.fit 1.2 mul out.fit         - multiply image by 1.1
- imarith 'in1.fit[1:20,1:20]' 'in2.fit[1:20,1:20]' add \!out.fit
  
    add 2 image sections; also overwrite out.fit if it exists.

- imarith data.fit[1] data.fit[2] mul out.fit - multiply the images

    in the 1st and 2nd extensions of the file data.fit

This program first opens the input images. If 2 images, it checks that
they have the same dimensions. It then creates the empty output file
and copies the header of the first image into it (thus duplicating the
size and data type of that image). Memory is allocated to hold one row
of the images. The program computes the output image values by reading
one row at a time from the input image(s), performing the desired
arithmetic operation, and then writing the resulting row to the output
file. The program reads and writes the row of data in double precision
format, regardless of the intrinsic datatype of the images; CFITSIO
transparently converts the data format if necessary as the rows are
read and written. This program also supports 3D data cubes by looping
through each plane of the cube.

2010-08-19  modified to allow numeric second argument 
   (contributed by Michal Szymanski, Warsaw University Observatory)

Version 1.0: September 29, 2017

  \author Dimitrios Psaltis (updated from 
https://heasarc.gsfc.nasa.gov/docs/software/fitsio/cexamples.html)
  
  \version 1.0
  
  \date September 29, 2017

  \bug No known bugs
  
  \warning no warnings
  
  \todo nothing left

*/
#include <string.h>
#include <stdio.h>
#include "fitsio.h"

int main(int argc, char *argv[])
{
    fitsfile *afptr, *bfptr, *outfptr;  /* FITS file pointers */
    int status = 0;  /* CFITSIO status value MUST be initialized to zero! */
    int atype, btype, anaxis, bnaxis, check = 1, ii, op;
    long npixels = 1, firstpix[3] = {1,1,1}, ntodo;
    long anaxes[3] = {1,1,1}, bnaxes[3]={1,1,1};
    double *apix, *bpix, value;
    int image2=1;
    
    if (argc != 5) { 
      printf("Usage: imarith image1 { image2 | value } oper outimage \n");
      printf("\n");
      printf("Perform 'image1 oper image2' or 'image1 oper value'\n");
      printf("creating a new output image.  Supported arithmetic\n");
      printf("operators are add, sub, mul, div (first character required\n");
      printf("\n");
      printf("Examples: \n");
      printf("  imarith in1.fits in2.fits a out.fits - add the 2 files\n");
      printf("  imarith in1.fits 1000.0 mul out.fits - mult in1 by 1000\n");
      return(0);
    }

    fits_open_file(&afptr, argv[1], READONLY, &status); /* open input images */
    if (status) {
       fits_report_error(stderr, status); /* print error message */
       return(status);
    }
    fits_open_file(&bfptr, argv[2], READONLY, &status);
    if (status) {
      value = atof(argv[2]);
      if (value == 0.0) {
	printf("Error: second argument is neither an image name"
	       " nor a valid numerical value.\n");
	return(status);
      }
      image2 = 0;
      status = 0;
    }

    fits_get_img_dim(afptr, &anaxis, &status);  /* read dimensions */
    if (image2) fits_get_img_dim(bfptr, &bnaxis, &status);
    fits_get_img_size(afptr, 3, anaxes, &status);
    if (image2) fits_get_img_size(bfptr, 3, bnaxes, &status);

    if (status) {
       fits_report_error(stderr, status); /* print error message */
       return(status);
    }

    if (anaxis > 3) {
       printf("Error: images with > 3 dimensions are not supported\n");
       check = 0;
    }
         /* check that the input 2 images have the same size */
    else if ( image2 && ( anaxes[0] != bnaxes[0] || 
			  anaxes[1] != bnaxes[1] || 
			  anaxes[2] != bnaxes[2] ) ) {
       printf("Error: input images don't have same size\n");
       check = 0;
    }

    if      (*argv[3] == 'a' || *argv[3] == 'A')
      op = 1;
    else if (*argv[3] == 's' || *argv[3] == 'S')
      op = 2;
    else if (*argv[3] == 'm' || *argv[3] == 'M')
      op = 3;
    else if (*argv[3] == 'd' || *argv[3] == 'D')
      op = 4;
    else {
      printf("Error: unknown arithmetic operator\n");
      check = 0;
    }

    /* create the new empty output file if the above checks are OK */
    if (check && !fits_create_file(&outfptr, argv[4], &status) )
    {
      /* copy all the header keywords from first image to new output file */
      fits_copy_header(afptr, outfptr, &status);

      npixels = anaxes[0];  /* no. of pixels to read in each row */

      apix = (double *) malloc(npixels * sizeof(double)); /* mem for 1 row */
      if (image2) bpix = (double *) malloc(npixels * sizeof(double)); 

      if (apix == NULL || (image2 && bpix == NULL)) {
        printf("Memory allocation error\n");
        return(1);
      }

      /* loop over all planes of the cube (2D images have 1 plane) */
      for (firstpix[2] = 1; firstpix[2] <= anaxes[2]; firstpix[2]++)
      {
        /* loop over all rows of the plane */
        for (firstpix[1] = 1; firstpix[1] <= anaxes[1]; firstpix[1]++)
        {
          /* Read both images as doubles, regardless of actual datatype.  */
          /* Give starting pixel coordinate and no. of pixels to read.    */
          /* This version does not support undefined pixels in the image. */

          if (fits_read_pix(afptr, TDOUBLE, firstpix, npixels, NULL, apix,
                            NULL, &status))
	    break;   /* jump out of loop on error */
	  if (image2 && fits_read_pix(bfptr, TDOUBLE, firstpix, npixels,
				      NULL, bpix, NULL, &status))
	    break;   /* jump out of loop on error */

          switch (op) {
          case 1:         
            for(ii=0; ii< npixels; ii++)
	      if (image2)
		apix[ii] += bpix[ii];
	      else
		apix[ii] += value;
            break;
          case 2:
            for(ii=0; ii< npixels; ii++)
	      if (image2)
		apix[ii] -= bpix[ii];
	      else
		apix[ii] -= value;
            break;
          case 3:
            for(ii=0; ii< npixels; ii++)
	      if (image2)
		apix[ii] *= bpix[ii];
	      else
		apix[ii] *= value;
            break;
          case 4:
            for(ii=0; ii< npixels; ii++) {
	      if (image2) {
		if (bpix[ii] !=0.)
		  apix[ii] /= bpix[ii];
		else
		  apix[ii] = 0.;
	      }
	      else {
		apix[ii] /= value;
	      }
            }
          }

          fits_write_pix(outfptr, TDOUBLE, firstpix, npixels,
                       apix, &status); /* write new values to output image */
        }
      }    /* end of loop over planes */

      fits_close_file(outfptr, &status);
      free(apix);
      if (image2) free(bpix);
    }

    fits_close_file(afptr, &status);
    if (image2) fits_close_file(bfptr, &status);
 
    if (status) fits_report_error(stderr, status); /* print any error message */
    return(status);
}

