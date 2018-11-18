#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fftw3.h>

#include "fitsio.h"
/*! \file
  \brief 
  Converts an image to u-v maps (complex amplitude and phase)

  \details
  This program reads an image stored in an input FITS file,
  calculates its complex Fourier transform, and stores the resulting
  visibility amplitudes and phases in an output FITS file.

  The dimension of the Fourier transform along each axis is the 
  maximum of the dimension of the image along the same axis and
  the optional number of padding points. 

  The output FITS file has two HDU images; the first one has the
  visibility amplitudes and the second one has the visibility phases
  (in rad). The center of the Fourier transform is at grid point (starting from 1)
  Nx/2 and Ny/2, where Nx (or Ny) is the maximum of the dimension of the
  image along the horizontal (vertical) axis and the optional number of 
  padding points.

  In order to avoid numerical issues, when the amplitude is smaller than 
  a predefined fraction (MINAMP) of the zero baseline amplitude, the phase
  is set to zero.

  Use: image2uv [-sv] [-p Npoints] [-c] [-o filename2] filename1

  The required options are:
  - "filename1": sets the input image filename (FITS)
  
  The optional options are:
  - "-o filename2": sets the output visibility filename (UVFITS).
  - "-s": silent mode. It does not print anything and uses defaults 
  - "-v": verbose mode. It prints a lot more information 
  - "-p Npoints": pads the image to a square grid with Npoints on each side, if 
the current image size is smaller than Npoints, before taking the Fourier Transform
  - "-c": calculates the complex phases by first centering the image to its center of brightness. If this options is not given, it calculates the complex phase with respect to the geometric center of the image.

  If no options are given, it prints a help message

  Examples:

  - image2uv -o uvresults.out inimage.fits 

  Reads the file form inimage.fits and outputs the visibility amplitudes and phases
  into uvresults.out
  
  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date September 30, 2017

  \bug No known bugs
  
  \warning No known warnings
  
  \todo Change call to FFTW to use only routines for Real data. 
  Add error capture if-statements for all the read and the write commands

*/
// Definitions

#define DEFAULTOUTFILENAME "uvout.fits"  //!< default filename for output, if -o option is not given
#define VMODEDEFAULT 1                   //!< default verbose mode "medium"
#define MAXCHAR 80                       //!< maximum number of characters for strings
#define RED "\x1B[31m"                   //!< color RED for error output
#define RESETCOLOR "\x1B[0m"             //!< color to reset to normal
#define MINAMP 1.e-12                    //!< minimum fraction of zero baseline amplitude, below which the phase is set to zero

/*!
\brief Prints an error message

\details 

\author Dimitrios Psaltis

\version 1.0

\date September 30, 2017

@param errmsg[] a string with the error message to be printed

\return nothing

*/
void printErrorImage2uv(char errmsg[])
{
  fprintf(stderr,RED "image2uv: %s" RESETCOLOR,errmsg);

  return;
}
  
/*!
\brief Prints a help message when no other arguments are given

\details 

\author Dimitrios Psaltis

\version 1.0

\date September 30, 2017

\pre It is called from parse() and from main()

@param no parameters

\return nothing

*/
void printhelp(void)
{
  printf("\n");
  printf("Reads an image stored in an input FITS file, calculates\n");
  printf("its complex Fourier transform, and stores the resulting\n");
  printf("visibility amplitudes and phases in an output FITS file.\n");
  printf("\n");
  printf("Use: image2uv [-sv] [-p Npoints] [-c] [-o <fname>] <fname> \n");
  printf("\n");
  printf("Options:\n");
  printf("\n");
  printf("-o <fname>: sets the output visibility filename.\n");
  printf("            The default is <uvout.fits>\n");
  printf("-s: silent mode. It does not print anything.\n");
  printf("-v: verbose mode. It prints a lot more information .\n");
  printf("-p Npoints: pads the image to a square grid with Npoints on each side, \n"); 
  printf("            if the current image size is smaller than Npoints, before taking\n");
  printf("            the Fourier Transform.\n");
  printf("-c: calculates the complex phases by first centering the image to its center of\n");
  printf("    brightness. If this options is not given, it calculates the complex phase with\n");
  printf("    respect to the geometric center of the image.\n");
  printf("\n");
}

/*!
\brief Asks for user input for all the options of this command

\details 
It asks the user to input directly all the possible options that one can 
give in this command. It is called from the main function if the '-v' option is
given as an argument.

When each option is presented, the default value or the value that was passed as an
argument is displayed. If the user hits return, then that value remains. If the user
provides a valid answer, it replaces the value for that option.

\author Dimitrios Psaltis

\version 1.0

\date November 12, 2017

\pre It is called from main()

@param *outFileName a string which provides and returns the output filename

@param *vmode an int with a flag for the chosen verbose mode (0:silent, 1: normal, 2: verbose)

@param *cmode an int with a flat for whether the FFT will be centered on the center of brightness

@param *Npad an int with the number of points per dimension to which the image will be padded. It it is smaller than the number of points in the image, then the total number of points in the image will be used.

\return Nothing

*/
void verboseinput(char *outFileName, int *vmode, int *cmode, int *Npad)
{
  int NpadInput;                 // new value for Npad
  char line[80];                 // line of characters
  char *ptr;                     // pointer used for converting strings to numbers
  char yesOrNo;                  // character for yes or no answers
  
  // ask for new padding number
  printf("Padding to Number of points [%d]: ",*Npad);
  fgets(line, sizeof line, stdin);
  NpadInput=strtol(line, &ptr, 10);
  if (NpadInput!=0)
    *Npad=NpadInput;
  
  // ask for brightness centering
  do
    {
      printf("Displace image to brightness center? [Y/n]: ");
      fgets(line, sizeof line, stdin);
      yesOrNo=line[0];
    }
  while (!(yesOrNo=='y' || yesOrNo=='Y' || yesOrNo=='n' || yesOrNo=='N' || yesOrNo=='\n'));
  if (yesOrNo=='n' || yesOrNo=='N')
    {
      *cmode=1;
    }
  else
    {
      *cmode=0;
    }

  // ask for output file
  printf("Output file name [%s]: ",outFileName);
  fgets(line, sizeof line, stdin);
  // do something only if it's not a newline
  if (line[0]!='\n')
    {
      // first remove trailing newline
      line[strcspn(line, "\n")] = 0;
      strcpy(outFileName,line);
    }
		      
  return;
}

/*!
\brief Parses the command line for options

\details 
Parses the command line for options. If no options are given,
it prints a help message

The required options are:
- <filename>: sets the input image filename (FITS)

The optional options are:
- "-o <filename>": sets the output visibility filename (UVFITS).
- "-s": silent mode. It does not print anything 
- "-v": verbose mode. It prints a lot more information
- "-p Npoints": pading. It pads the image to a square grid with Npoints on each side
- "-c": calculates the complex phase by first centering the image to its center of brightness.

\author Dimitrios Psaltis

\version 1.0

\date September 30, 2017

\pre It is called from main()

@param argc an int (as is piped from the unix prompt)

@param argv[] an array of strings (as is piped from the unix prompt)

@param *inFileName a string which returns the input filename

@param *outFileName a string which returns the output filename

@param *vmode an int with a flag for the chosen verbose mode (0:silent, 1: normal, 2: verbose)

@param *cmode an int with a flat for whether the FFT will be centered on the center of brightness

@param *Npad an int with the number of points per dimension to which the image will be padded. It it is smaller than the number of points in the image, then the total number of points in the image will be used.

\return Returns zero if successful, 1 if not

*/
int parse(int argc, char *argv[], char *inFileName, char *outFileName, int *vmode, int *cmode, int *Npad)
{
  int opt = 0;
  int index;
  
  opterr=0;            // do not print any other errors
  
  if (argc==1)         // if no options are given
    {
      printhelp();     // print help message and return with a code to do nothing
      return 1;
    }

  *vmode=VMODEDEFAULT;                      // default verbose mode "medium"
  strcpy(outFileName,DEFAULTOUTFILENAME);   // default filename for output file
  
  // parse through arguments with options
  while ((opt = getopt(argc, argv, "o:svcp:")) != -1)
    {
      switch(opt)
	{
	case 'o':
	  strcpy(outFileName,optarg);
	  break;
	case 's':
	  *vmode=0;                         // verbose mode "silent"
	  break;
	case 'v':
	  *vmode=2;                         // verbose mode "verbose"
	  break;
	case 'c':
	  *cmode=1;                         // centering mode is on
	  break;
	case 'p':                           // if padding is introduced
	  *Npad=strtoumax(optarg, NULL, 10);// return number of padded points
	  if (*Npad==0)
	    {
	      printErrorImage2uv("Invalid number of padding points\n");
	      return 1;
	    }
	  break;
	case '?':
	    {
	      printErrorImage2uv("Invalid option received\n");
	    }
	  break;
	}
    }

  // The variable argc counts the total number of elements in the input,
  //    while the variable optind counts the number of arguments
  //    that have been parsed. If the latter is larger than the
  //    former, then there is no argument after the options

      if (optind >= argc) {
	printErrorImage2uv("Expected argument after options\n");
        return 1;
  }

  if (argc-optind!=1)  // it requires at least one argument with no options
    {
      printErrorImage2uv("Too many arguments\n");
      return 1;
    }
  
  // input file name is the single non-option argument
  strcpy(inFileName,argv[optind]);
  
  return 0; 
}

/*!
 \brief Main program

 \author Dimitrios Psaltis

 \version 1.0

 \date September 30, 2017

 \pre Nothing

 */
int main(int argc, char *argv[])
{
  int index;                                        // generic integer variable
  char inFileName[MAXCHAR], outFileName[MAXCHAR];   // string for input and output filenames
  char hist[MAXCHAR];                               // string for history in output FITS file
  int vmode;                                        // flag for verbose mode
  int cmode;                                        // flag for image centering
  int Npad=0;                                       // Number of points per dimension for image padding;
  int iColStart,iRowStart;                          // Startng row and column of padded image
  int NxPad, NyPad;                                 // Size of padded image in 2D
  
  int Nx,Ny;                                        // size of image in 2D (to be read from file)
  double xScale,yScale;                             // physical sizes of image pixels along the two directions
  double *ImageIn;                                  // pointer to image array

  double *Va, *Vp;                                  // pointers to arrays with amplitude and phase
                                                    // of complex visibilities
  double vScale,uScale;                             // physical sizes of u-v pixels along the two directions

  double fluxXCent=0.0, fluxYCent=0.0;              // variables for finding the brightness center
  double fluxTotal=0.0;                             // total flux in the image (arb units)
  
  fftw_complex *in, *out;                           // pointers to arrays for 2D FFTs
  fftw_plan p;                                      // 2D fft plan used in FFTW

  int indexR,indexC;                                // dummy indices for counting rows and columns

  int dummyResult;                                  // dummy variable for integer results of functions
	  
  // parse the command line
  int parseflag=parse(argc, argv,&inFileName,&outFileName,&vmode,&cmode,&Npad);

  // if there was an error in parsing, return with an error code
  if (parseflag!=0) return 1;

  // read the size (in 2D) of the input file
  int readflag=readFITSImagedim(inFileName, &Ny, &Nx,&yScale,&xScale);

  // if there was no physical scale in the image, just set it to unity
  if (xScale==0 || yScale==0)
    {
      xScale=1.0;
      yScale=1.0;
    }
    
  // if there is a problem
  if (readflag!=0)
    {
      printErrorImage2uv("reading file failed!\n");   // print error message
      return 1;                                     // return with error code
    }

  // if in verbose mode, ask for all the inputs again
  if (vmode==2)
    verboseinput(outFileName, &vmode, &cmode, &Npad);

  // figure out padding
  dummyResult=ArrayPad(Ny, Nx, Npad, &iRowStart, &iColStart, &NyPad, &NxPad);  

  // allocate memory for the image and visibility arrays
  ImageIn = (int *)malloc(sizeof(double)*NxPad*NyPad);  // allocate memory to store image
  Va = (int *)malloc(sizeof(double)*NxPad*NyPad);  // allocate memory to store Vis Amplitude
  Vp = (int *)malloc(sizeof(double)*NxPad*NyPad);  // allocate memory to store Vis Phase
  
  // if memory allocation failed
  if(ImageIn == NULL)
    {
      printErrorImage2uv("malloc failed!\n");   // print error message
      return 1;                               // return with error code
    }
  
  // now read the whole file
  readflag=readFITSImage(inFileName, Ny, Nx, Npad,ImageIn);
  
  // if there was a problem
  if (readflag!=0)
    {
      printErrorImage2uv("reading file failed!\n");   // print error message
      return 1;                                     // return with error code
    }

  if (vmode!=0)
    printf("image2uv: Read %dx%d image from file %s\n",Nx,Ny,inFileName);
  
  // allocate memory for the image to FFT
  in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * NxPad* NyPad);
  // allocate memory for the output of the FFT
  out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * NxPad* NyPad);
  
  // make a 2D FFTW plan, as required by the FFTW library
  // HERE!!!!
  p = fftw_plan_dft_2d(NyPad,NxPad, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
  
  // fill the input array using the image that was just read
  // and, in the meantime, find the brightness center of the image
  for (indexR=1;indexR<=NyPad;indexR++)
    {
      for (indexC=1;indexC<=NxPad;indexC++)
	{
	  index=indexArr(indexR,indexC,NyPad,NxPad);
	  in[index][0]=*(ImageIn+index);
	  in[index][1]=0.0;                  // no imaginary part
	  fluxXCent+=indexC*in[index][0];    // add to calculate center of brightness
	  fluxYCent+=indexR*in[index][0];    // add to calculate center of brightness
	  fluxTotal+=in[index][0];           // add for total flux in the image
	}
    }

  // calculate the flux center
  // if there is some flux in the image and the centering option is on
  if (fluxTotal!=0.0 && cmode==1)
    {
      // normalize the x- and y- coordinates
      fluxXCent/=fluxTotal;
      fluxYCent/=fluxTotal;
      if (vmode!=0)
	printf("image2uv: brightness center at the (%7.1f,%7.1f) grid point\n",fluxXCent,fluxYCent);
    }
  else
    {
      // otherwise just center it
      fluxXCent=NxPad/2.0;
      fluxYCent=NyPad/2.0;
    }

  // calculate the FFT of the image based on the FFTW plan
  fftw_execute(p);
  // destroy the FFTW plan
  fftw_destroy_plan(p);

  if (vmode!=0)
    printf("image2uv: FFT of the padded %dx%d image completed\n",NxPad,NyPad);

  // Convert the complex FFT to visibility amplitudes and phases
  // Also transpose the FFT array so that it is centered.

  // first keep the zero baseline amplitude
  double zeroBaselineAmp=sqrt(out[0][0]*out[0][0]+out[0][1]*out[0][1]);

  if (vmode!=0)
    printf("image2uv: zero baseline amplitude is %e\n",zeroBaselineAmp);
  
  // go through all rows
  for (indexR=1;indexR<=NyPad;indexR++)
    {
      // and all columns
      for (indexC=1;indexC<=NxPad;indexC++)
	{
	  // transpose the array to have it centered
	  if (indexR<=NyPad/2 && indexC<=NxPad/2)
	    {
	      index=indexArr(indexR+NyPad/2,indexC+NxPad/2,NyPad,NxPad);
	    }
	  if (indexR<=NyPad/2 && indexC>NxPad/2)
	    {
	      index=indexArr(indexR+NyPad/2,indexC-NxPad/2,NyPad,NxPad);
	    }
	  if (indexR>NyPad/2 && indexC<=NxPad/2)
	    {
	      index=indexArr(indexR-NyPad/2,indexC+NxPad/2,NyPad,NxPad);
	    }
	  if (indexR>NyPad/2 && indexC>NxPad/2)
	    {
	      index=indexArr(indexR-NyPad/2,indexC-NxPad/2,NyPad,NxPad);
	    }
	  // now calculate the index of the folded array
	  int indexTo=indexArr(indexR,indexC,NyPad,NxPad);

	  // and store it in the appropriate place in the Amplitude and Phase arrays
	  Va[indexTo]=sqrt(out[index][0]*out[index][0]+out[index][1]*out[index][1]);

	  // if the amplitude is too small, set the phase to zero
	  if (zeroBaselineAmp!=0 && fabs(Va[indexTo]/zeroBaselineAmp)<MINAMP)
	    {
	      Vp[indexTo]=0.0;
	    }   
	  else     // otherwise calculate it
	    {
	      Vp[indexTo]=atan2(out[index][1],out[index][0]);
	      
	      // make sure you add to the phase the displacement to the appropriate center
	      double addPhase=Vp[indexTo]+2.*M_PI*(fluxXCent-1)*(indexC-1-NxPad/2)/NxPad
		+2.*M_PI*(fluxYCent-1)*(indexR-1-NyPad/2)/NyPad;
	      Vp[indexTo]=atan2(sin(addPhase),cos(addPhase));
	    }
	  // *** Debugging only
	  //	  printf ("%d %d %e %e\n",indexR,indexC,Va[indexTo],Vp[indexTo]);
	}      
    }

  // calculate scale of pixels in u-v plane (the scales in the image are in degrees, so they need also
  // to be converted to rad.
  uScale=180.0/(NxPad*xScale*M_PI);
  vScale=180.0/(NyPad*yScale*M_PI);
  
  // create a history string to include in the FITS output
  strcpy(hist,"Created from Image in File: ");
  strcat(hist,inFileName);
  
  int writeflag=writeFITSVis(outFileName,NyPad,NxPad, Vp, Va, vScale,uScale,hist);

  // free the allocated memory
  fftw_free(in);
  fftw_free(out);
  free(ImageIn);
  free(Va);
  free(Vp);

  if (writeflag==1)
    {
      return 1;
    }
  
  if (vmode!=0)
    printf("image2uv: Wrote visibility amplitudes and phases to file %s\n",outFileName);

  return 0;                                          // normal return
}
