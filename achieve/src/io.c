#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define RED "\x1B[31m"                   //!< color RED for error output
#define RESETCOLOR "\x1B[0m"             //!< color to reset to normal

#include "fitsio.h"
/*! \file
  \brief 
  Subroutines to perform I/O with FITS, OIFITS, etc. files

  \details

  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date October 2, 2017

  \bug No known bugs
  
  \warning No known warnings
  
  \todo nothing left

*/

/*!
\brief Prints an error message

\details 

\author Dimitrios Psaltis

\version 1.0

\date September 30, 2017

@param errmsg[] a string with the error message to be printed

\return nothing

*/
void printErrorIO(char errmsg[])
{
  fprintf(stderr,RED "io: %s" RESETCOLOR,errmsg);

  return;
}

/*!
  \brief 
  Reads the image dimensions of a FITS file, both in terms of pixels and in terms
  of physical units.

  \details
  Reads the image dimensions of the FITS file given in the argument fname. It 
  returns the dimensions of the image (in pixels) along its two axes in
  Nx and Ny and the size of each pixel along the two axis in xScale and yScale.

  In this and in other subroutines, the x-axis is the same as the columns
  and the y-axis is the same as the rows of the image. These two notations
  will be used interchangeably.

  It returns zero if everything was OK or the FITS error code (and prints
  an error message) if it wasn't.

  @param fname[] a string with the filename to be read
  @param *Ny on return, an int pointer with the dimension of the "y-axis" (# of rows)
  @param *Nx on return, an int pointer with the dimension of the "x-axis" (# of columns)
  @param *yScale on return, a double pointer with the physical size of a pixel along the y-axis 
  @param *xScale on return, a double pointer with the physical size of a pixel along the x-axis

  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date October 2, 2017

  \bug No known bugs
  
  \warning No known warnings
  
  \todo nothing left

*/
int readFITSImagedim(char fname[], int *Ny, int *Nx, double *yScale, double *xScale)
{
  fitsfile *fptr;   // FITS file pointer, defined in fitsio.h
  int status = 0;   // CFITSIO status value MUST be initialized to zero! 

  int bitpix;              // data type for pixel values
  int naxis;               // number of axes
  long naxes[2] = {1,1};   // dimension of each axis
  char keyname[80],value[80],comment[80]; // strings for reading keywords from FITS file
  char *ptr;                     // pointer used for converting strings to numbers

  // open file as READONLY
  if (!fits_open_file(&fptr, fname, READONLY, &status))
    {
      // get the parameters of the image
      if (!fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status) )
        {
          if (naxis!=2)   // we will only be using 2D images
	    printErrorIO("readFITS: only 2D images are supported\n");
	  else
	    {
	      *Nx=naxes[0];      // naxes[0] are C-like columns
	      *Ny=naxes[1];
	    }
	}

      // read the physical sizes of the pixels, stored in Keywords "CDELT1" and "CDELT2"
      // start from the first dimension
      strcpy(keyname,"CDELT1");
      if (!fits_read_key_str(fptr, keyname, value, comment, &status))
	{
	  *xScale=strtod(value, &ptr);
	}

      // continue with the second dimension
      strcpy(keyname,"CDELT2");
      if (!fits_read_key_str(fptr, keyname, value, comment, &status))
	{
	  *yScale=strtod(value, &ptr);
	}

      fits_close_file(fptr, &status);   // close image file for now
    } 
  
  // print any error message
  if (status) fits_report_error(stderr, status); 

  return(status);
}

/*!
  \brief 
  Reads the image stored in a FITS file and, optionally, pads it with zeros.

  \details
  Reads the image stored in the FITS file 'fname', with known dimensions
  Nx and Ny [to be obtained using readFITSdim()]. If Npad is larger than Nx or Ny,
  then it pads the image so that the corresponding dimension has Npad grid points.

  It returns zero if everything was OK or the FITS error code (and prints
  an error message) if it wasn't.

  @param fname[] a string with the filename to be read
  @param Nx an int with the dimension of the "x-axis"
  @param Ny an int with the dimension of the "y-axis"
  @param Npad an int with the dimension along each direction of the padded image

  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date October 2, 2017

  \bug No known bugs
  
  \warning No known warnings
  
  \todo nothing left

*/
int readFITSImage(char fname[], int Ny, int Nx, int Npad, double *Image)
{
  fitsfile *fptr;   // FITS file pointer, defined in fitsio.h
  int status = 0;   // CFITSIO status value MUST be initialized to zero! 

  int bitpix;       // data type for pixel values
  int naxis;        // number of axes
  long naxes[2] = {1,1};   // dimension of each axis
  long fpixel[2] = {1,1};  // pixel counter
  int iRowStart,iColStart; // starting grid point at which to place the image, if padding is present
  int NxPad,NyPad;         // size of padded image array

  int dummyResult;         // dummy variable for the integer result of functions
  double doubleType;       // dummy double variable to calculate its size
  
  // open file as READONLY
  if (!fits_open_file(&fptr, fname, READONLY, &status))
    {
      // get the parameters of the image
      if (!fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status) )
        {
          if (naxis!=2)   // we will only be using 2D images
	    printErrorIO("readFITS: only 2D images are supported\n");
          else
	    {
	      if (Nx!=naxes[0])          // check if x-size is as stated in the input
		{
		  printErrorIO("readFITS: error in image x-dimension\n");
		  return 1;
		}
	      if (Ny!=naxes[1])          // check if y-size is as stated in the input
		{
		  printErrorIO("readFITS: error in image y-dimension\n");
		  return 1;
		}
	    }
	}

      // calculate padding
      dummyResult=ArrayPad(Ny, Nx, Npad, &iRowStart, &iColStart, &NyPad, &NxPad);

      // start at the beginning of the saved image
      fpixel[0]=1;
      fpixel[1]=1;

      // and read the pixels one row at a time, in order to put them in the right place
      // in the padded image

      for (fpixel[1] = 1; fpixel[1]<=Ny; fpixel[1]++)
	{
	  if (fits_read_pix(fptr, TDOUBLE, fpixel, Nx, NULL,
			    Image+indexArr(iRowStart+fpixel[1]-1,iColStart,NyPad,NxPad)
			    , NULL, &status) )
	    {
	      printErrorIO("readFITS: error in reading file\n");
	    }
	  
	}

      // close the file
      fits_close_file(fptr, &status);   // close image file for now
    }

  // print any error message
  if (status) fits_report_error(stderr, status); 
  
  return(status);
}

/*!
  \brief 
  Reads the image dimensions of a FITS data cube, both in terms of
  pixels and in terms of physical units.

  \details 
  Reads the image dimensions of the FITS file given in the argument
  fname. It returns the dimensions of the image (in pixels) along its
  two axes in Nx and Ny and the size of each pixel along the two axis
  in xScale and yScale.
  
  The difference with readFITSImageDim is that this subroutine reads
  in a 4D cube. It only works with FITS files that have a dimension of
  4, and assumes that the first two directions are the actual image
  axes.

  In this and in other subroutines, the x-axis is the same as the columns
  and the y-axis is the same as the rows of the image. These two notations
  will be used interchangeably.

  It returns zero if everything was OK or the FITS error code (and prints
  an error message) if it wasn't.

  @param fname[] a string with the filename to be read
  @param *Ny on return, an int pointer with the dimension of the "y-axis" (# of rows)
  @param *Nx on return, an int pointer with the dimension of the "x-axis" (# of columns)
  @param *yScale on return, a double pointer with the physical size of a pixel along the y-axis 
  @param *xScale on return, a double pointer with the physical size of a pixel along the x-axis

  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date July 30, 2018

  \bug No known bugs
  
  \warning No known warnings
  
  \todo nothing left

*/
int readFITSCubedim(char fname[], int *Ny, int *Nx, double *yScale, double *xScale)
{
  fitsfile *fptr;   // FITS file pointer, defined in fitsio.h
  int status = 0;   // CFITSIO status value MUST be initialized to zero! 

  int bitpix;              // data type for pixel values
  int naxis;               // number of axes
  long naxes[4] = {1,1,1,1};   // dimension of each axis
  char keyname[80],value[80],comment[80]; // strings for reading keywords from FITS file
  char *ptr;                     // pointer used for converting strings to numbers

  // open file as READONLY
  if (!fits_open_file(&fptr, fname, READONLY, &status))
    {
      // get the parameters of the image
      if (!fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status) )
        {
          if (naxis!=4)   // we will only be using 4D cubes
	    printErrorIO("readFITScube: only 4D cubes are supported\n");
	  else
	    {
	      *Nx=naxes[0];      // naxes[0] are C-like columns
	      *Ny=naxes[1];
	    }
	}

      // read the physical sizes of the pixels, stored in Keywords "CDELT1" and "CDELT2"
      // start from the first dimension
      strcpy(keyname,"CDELT1");
      if (!fits_read_key_str(fptr, keyname, value, comment, &status))
	{
	  *xScale=strtod(value, &ptr);
	}

      // continue with the second dimension
      strcpy(keyname,"CDELT2");
      if (!fits_read_key_str(fptr, keyname, value, comment, &status))
	{
	  *yScale=strtod(value, &ptr);
	}

      fits_close_file(fptr, &status);   // close image file for now
    } 
  
  // print any error message
  if (status) fits_report_error(stderr, status); 

  return(status);
}

/*!
  \brief 
  Reads the image stored in a FITS data cube and, optionally, pads it with zeros.

  \details
  Reads the image stored in the FITS file 'fname', with known
  dimensions Nx and Ny [to be obtained using readFITSdim()]. If Npad
  is larger than Nx or Ny, then it pads the image so that the
  corresponding dimension has Npad grid points.

  The difference with readFITSImage is that this subroutine reads
  in a 4D cube. It only works with FITS files that have a dimension of
  4, and assumes that the first two directions are the actual image
  axes.

  It returns zero if everything was OK or the FITS error code (and prints
  an error message) if it wasn't.

  @param fname[] a string with the filename to be read
  @param Nx an int with the dimension of the "x-axis"
  @param Ny an int with the dimension of the "y-axis"

  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date July 30, 2018

  \bug No known bugs
  
  \warning No known warnings
  
  \todo nothing left

*/
int readFITSCube(char fname[], int Ny, int Nx, double *Image)
{
  fitsfile *fptr;   // FITS file pointer, defined in fitsio.h
  int status = 0;   // CFITSIO status value MUST be initialized to zero! 

  int bitpix;       // data type for pixel values
  int naxis;        // number of axes
  long naxes[4] = {1,1,1,1};   // dimension of each axis
  long fpixel[4] = {1,1,1,1};  // pixel counter
  int iRowStart,iColStart; // starting grid point at which to place the image, if padding is present
  int dummyResult;         // dummy variable for the integer result of functions
  double doubleType;       // dummy double variable to calculate its size
  
  // open file as READONLY
  if (!fits_open_file(&fptr, fname, READONLY, &status))
    {
      // get the parameters of the image
      if (!fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status) )
        {
          if (naxis!=4)   // we will only be using 4D images
	    printErrorIO("readFITScube: only 4D images are supported\n");
          else
	    {
	      if (Nx!=naxes[0])          // check if x-size is as stated in the input
		{
		  printErrorIO("readFITScube: error in image x-dimension\n");
		  return 1;
		}
	      if (Ny!=naxes[1])          // check if y-size is as stated in the input
		{
		  printErrorIO("readFITScube: error in image y-dimension\n");
		  return 1;
		}
	    }
	}

      // start at the beginning of the saved image
      fpixel[0]=1;
      fpixel[1]=1;

      // and read the pixels one row at a time, in order to put them in the right place
      // in the image; it is assumed that the first layer is the image

      for (fpixel[1] = 1; fpixel[1]<=Ny; fpixel[1]++)
	{
	  if (fits_read_pix(fptr, TDOUBLE, fpixel, Nx, NULL,
			    Image+indexArr(iRowStart+fpixel[1]-1,iColStart,Ny,Nx)
			    , NULL, &status) )
	    {
	      printErrorIO("readFITS: error in reading file\n");
	    }
	  
	}

      // close the file
      fits_close_file(fptr, &status);   // close image file for now
    }

  // print any error message
  if (status) fits_report_error(stderr, status); 
  
  return(status);
}

/*!
  \brief 
  Writes visibility amplitudes and phases into a FITS file

  \details
  Given two real arrays Va[] and Vp[] of dimensions Nx by Ny, it stores
  them in the FITS file 'fname'

  It returns zero if everything was OK or the FITS error code (and prints
  an error message) if it wasn't.

  @param fname[] a string with the filename to be read
  @param Nx an int with the dimension of the "x-axis"
  @param Ny an int with the dimension of the "y-axis"
  @param Va[] is a Nx by Ny double array with the visibility amplitudes
  @param Vp[] is a Nx by Ny double array with the visibility phases (in rad)
  @param uScale is a double with the physical size of each pixel in the x-direction
  @param vScale is a double with the physical size of each pixel in the x-direction
  @param hist[] is a string of characters to be put in the "history" field of the FITS file

  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date November 3, 2017

  \bug No known bugs
  
  \warning No known warnings
  
  \todo nothing left

*/
int writeFITSVis(char fname[], int Ny, int Nx, double *Vp, double *Va, double vScale, double uScale, char hist[])
{
  // output the results into a new FITS file
  fitsfile *fptr;   // FITS file pointer, defined in fitsio.h
  int status = 0;   // CFITSIO status value MUST be initialized to zero! 
  long naxes[2] = {1,1};   // dimension of each axis
  long fpixel[2] = {1,1};  // pixel counter
  char keyname[80],comment[80]; // strings for writing keywords to the FITS file
  int writeflag;    // flag for return values of FITS commands

  // set axes dimensions from input parameters
  naxes[0]=Nx;
  naxes[1]=Ny;

  // open file
  if (!fits_create_file(&fptr, fname, &status))
    {

      // create a FITS image configuration for the Visibility Amplitude
      fits_create_img(fptr,DOUBLE_IMG,2,naxes, &status);

      // Write the Visibility Amplitudes
      int writeflag=fits_write_pix(fptr, TDOUBLE, fpixel,Nx*Ny, Va, &status);

      // write the scale along the u-orientation
      strcpy(keyname,"CDELT1");
      writeflag+=fits_write_key_dbl(fptr,keyname,uScale,6, "in wavelengths",&status);
      // write the scale along the u-orientation
      strcpy(keyname,"CDELT2");
      writeflag+=fits_write_key_dbl(fptr,keyname,vScale,6, "in wavelengths",&status);
      
      // delete two standard comments
      writeflag+=fits_delete_key(fptr, "COMMENT", &status);
      writeflag+=fits_delete_key(fptr, "COMMENT", &status);

      // add three new comments, showing that it's the amplitudes, the history, and the date
      writeflag+=fits_write_comment(fptr, "Visibility Amplitudes", &status);
      writeflag+=fits_write_history(fptr, hist, &status);
      writeflag+=fits_write_date(fptr, &status);

      // create a FITS image configuration for the Visibility Phases
      fits_create_img(fptr,DOUBLE_IMG,2,naxes, &status);

      // write the visibility phases
      writeflag+=fits_write_pix(fptr, TDOUBLE, fpixel,Nx*Ny, Vp, &status);

      // write a new comment that this is about the phases
      writeflag+=fits_write_comment(fptr, "Visibility Phases", &status);

      // if any of these failed, writeflag will be non zero
      if (writeflag!=0)
	{
	  printErrorIO("writing output file failed!\n");   // print error message
	  return 1;                                     // return with error code
	}
      
      // close the output file
      fits_close_file(fptr, &status);   // close image file for now

    }
  else
    {
      printErrorIO("writing output file failed! Perhaps output file already exists\n");   // print error message
      return 1;                                     // return with error code
    }

  // print any error message
  if (status) fits_report_error(stderr, status); 
  
  return(status);
}

/*!
  \brief 
  Writes a model image into a FITS file

  \details
  Given the double array Image of dimensions Npixel by Npixel, it stores
  it in the FITS file 'fname'

  It returns zero if everything was OK or the FITS error code (and prints
  an error message) if it wasn't.

  @param fname[] a string with the filename to be read
  @param Npixel an int with the number of pixels along each axis
  @param pixelSize a double with the physical size of each pixel
  @param Image[] a Npixel by Npixel double array with the image
  @param hist[] a string of characters to be put in the "history" field of the FITS file

  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date November 3, 2017

  \bug No known bugs
  
  \warning No known warnings
  
  \todo nothing left

*/
int writeFITSImage(char fname[], int Npixel, double pixelSize, double *Image, char hist[])
{
  fitsfile *fptr;   // FITS file pointer, defined in fitsio.h
  int status = 0;   // CFITSIO status value MUST be initialized to zero! 
  long naxes[2] = {1,1};   // dimension of each axis
  long fpixel[2] = {1,1};  // pixel counter
  char keyname[80],comment[80]; // strings for writing keywords to the FITS file
  int writeflag;    // flag for return values of FITS commands

  // set axes dimensions from input parameters
  naxes[0]=Npixel;
  naxes[1]=Npixel;

  // open file
  if (!fits_create_file(&fptr, fname, &status))
    {

      // create a FITS image configuration for the Image
      fits_create_img(fptr,DOUBLE_IMG,2,naxes, &status);

      // Write the Image
      int writeflag=fits_write_pix(fptr, TDOUBLE, fpixel,Npixel*Npixel, Image, &status);

      // write the scale along the x-orientation
      strcpy(keyname,"CDELT1");
      writeflag+=fits_write_key_dbl(fptr,keyname,pixelSize,6, "in degrees",&status);
      // write the scale along the y-orientation
      strcpy(keyname,"CDELT2");
      writeflag+=fits_write_key_dbl(fptr,keyname,pixelSize,6, "in degrees",&status);
      
      // delete two standard comments
      writeflag+=fits_delete_key(fptr, "COMMENT", &status);
      writeflag+=fits_delete_key(fptr, "COMMENT", &status);

      // add two new comments, showing the history, and the date
      writeflag+=fits_write_history(fptr, hist, &status);
      writeflag+=fits_write_date(fptr, &status);

      // if any of these failed, writeflag will be non zero
      if (writeflag!=0)
	{
	  printErrorIO("writing output file failed!\n");   // print error message
	  return 1;                                     // return with error code
	}
      
      // close the output file
      fits_close_file(fptr, &status);   // close image file for now

    }
  else
    {
      printErrorIO("writing output file failed! Perhaps output file already exists\n");   // print error message
      return 1;                                     // return with error code
    }

  // print any error message
  if (status) fits_report_error(stderr, status); 
  
  return(status);
}

/*!
  \brief 
  Function to convert 2D array indices to a pointer location

  \details
  Given an [Ny][Nx] array, it takes the two coordinates i and j of an
  element and converts it to an incremental pointer location. The 
  i- and j- coordinates start from (1,1) for the first point

  @param i an int with the row coordinate of an element
  @param j an int with the column coordinate of an element
  @param Ny an int with the number of rows in the image (not used)
  @param Nx an int with the number of columns in the image

  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date October 3, 2017

  \bug No known bugs
  
  \warning No known warnings
  
  \todo nothing left

*/
int indexArr(int i, int j, int Ny, int Nx)
{
  return (i-1)*Nx+j-1;
}

/*!
  \brief 
  Function to calculate sizes and starting locations of padded images

  \details
  Given an initial size Ny*Nx of an immage array and a padding integer Npad, it
  returns (i) the starting row (iRowStart) and column (iColStart) of the actual
  image in the padded array (with the first pixel counting from [1,1])
  (ii) the size of the padded array along each direction (NxPad and NyPad), 
  which will be the larger of the initial size and Npad, along each direction

  It returns zero if it all worked out, one if it didn't.

  @param Ny an int with the number of rows in the image
  @param Nx an int with the number of columns in the image
  @param Npad an int with the padding parameter
  @param *iRowStart an int pointer that returns the starting row of the image in the padded array
  @param *iColStart an int pointer that returns the starting column of the image in the padded array
  @param NyPad an int pointer that returns the number of rows of the padded image
  @param NxPad an int pointer that returns the number of columns of the padded image

  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date October 3, 2017

  \bug No known bugs
  
  \warning it assumes that all integers are positive, so it never returns anything but zero
  
  \todo nothing left

*/
int ArrayPad(int Ny, int Nx, int Npad, int *iRowStart, int *iColStart, int *NyPad, int *NxPad)
{

  // calculate array start and size, given the padding
  if (Npad>Nx)                        // if there is padding in the x-direction
    {
      *iColStart=(Npad-Nx)/2+1;        // column at which image starts
      *NxPad=Npad;                     // x size of padded array
    }
  else
    {
      *iColStart=1;                    // if not, use defaults
      *NxPad=Nx;
    }
  if (Npad>Ny)                        // if there is padding in the y-direction
    {
      *iRowStart=(Npad-Ny)/2+1;       // row at which image starts
      *NyPad=Npad;                    // y size of padded array
    }
  else
    {
      *iRowStart=1;                   // if not, use defaults
      *NyPad=Ny;
    }

  return 0;
}

/*

int main(void)
{
  int Nx,Ny;
  double *ImageIn;
  
  int a=readFITSdim("18000467.fits", &Nx, &Ny);

  if (a==0) printf("!%d %d\n",Nx,Ny);
  
  ImageIn = (int *)malloc(sizeof(double)*Nx*Ny);  // allocate memory to store image

  if(ImageIn == NULL)
    {
      printf(":malloc failed!\n");   // print error message
      return 1;                      // return with error code
    }

  a=readFITS("18000467.fits", Nx, Ny, ImageIn);

  int i,j;

  for (i=1;i<=Nx;i++)
    {
      for (j=1;j<=Ny;j++)
	{
	  printf("%d %d %e\n",i,j,*(ImageIn+indexArr(i,j,Nx,Ny)));
	}
    }
  
  
	  
  return 0;
}

*/
