#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#include "fitsio.h"
/*! \file
  \brief 
  Creates a synthetic static image based on a model

  \details
  This program creates a synthetic static square image from a model 
  and stores the result in an output FITS file.

  Use: synthimage [-sv] -p Npixels -c size -m modelname -d param1,param2,... filename

  The required option is:
  - "filename": sets the output image filename (FITS)

  The optional options are:
  - "-p Npixels": sets the number of image pixels per dimension (default 512)
  - "-c size": physical dimension of each pixel in microarcsec (default 1.0)
  - "-m modelname": the name of the model to be used (default "gauss")
  - "-d param1,param2,...": the values of the various model parameters (separated by commas, with no spaces between them or in quotes) (default 1,0.0,0.0,20.0,20.0)
  - "-s": silent mode. It does not print anything and uses defaults 
  - "-v": verbose mode. It prints a lot more information 

  If no options are given, it prints a help message

  Examples:

  - synthimage -p 512 -c 1.0 -m gauss -d 1,0.,0.,10.0,2.0 image.fits 

  Creates a synthetic image with 512 pixels along its side, with each pixel having a 
  physical dimension of 1 microarcsec. The image is created from a gaussian model, with 
  one gaussian component, centered at (0.0,0.0) microarcsec from the center of the image
  and with standard deviation equal to 10.0 and 2.0 microarcsec along the x- and y- 
  orientations.
  
  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date November 12, 2017

  \bug No known bugs
  
  \warning No known warnings
  
  \todo Nothing to do

*/
// Definitions

#define VMODEDEFAULT 1                     //!< default verbose mode "medium"
#define MAXCHAR 80                         //!< maximum number of characters for strings
#define MAXPIXEL 4096                      //!< maximum number of pixels per size of image
#define NPIXELDEFAULT 512                  //!< default number of pixels
#define PIXELSIZEDEFAULT 1.0               //!< default pixel size
#define MODELDEFAULT "gauss"               //!< default model
#define PARAMDEFAULTG "1,1.0,0.0,0.0,20.0,20.0,0." //!< default parameter string for gaussian model
#define PARAMDEFAULTC "1,1.0,0.0,0.0,10.0,0.5,0.5,0." //!< default parameter string for crescent model
#define MAXPARAM 20                        //!< maximum number of model parameters

#define RED "\x1B[31m"                     //!< color RED for error output
#define RESETCOLOR "\x1B[0m"               //!< color to reset to normal

#define muarcsecToDegrees 2.777778e-10     //!< 1 microarcsec in degrees

/*!
\brief Prints an error message

\details 

\author Dimitrios Psaltis

\version 1.0

\date November 12, 2017

@param errmsg[] a string with the error message to be printed

\return nothing

*/
void printErrorSynthimage(char errmsg[])
{
  fprintf(stderr,RED "synthimage: %s" RESETCOLOR,errmsg);

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
    printf("This program creates a synthetic static square image from a model\n");
    printf("and stores the result in an output FITS file.\n");
    printf("\n");
    
    printf("Use:\n");
    printf("  synthimage [-sv] -p Npixels -c size -m modelname -d param1,param2,... filename\n");
    printf("\n");
    printf("The required option is:\n");
    printf("filename: sets the output image filename (FITS)\n");
    printf("\n");
    printf("The optional options are:\n");
    printf(" -p Npixels: sets the number of image pixels per dimension (default: 512)\n");
    printf(" -c  size: physical dimension of each pixel in microarcsec (default: 1.0)\n");
    printf(" -m modelname: the name of the model to be used (default: gauss)\n");
    printf(" -d param1,param2,...: the values of the various model parameters (separated\n");
    printf("                       by commas, with no spaces between them or in quotes)\n");
    printf("                       (default 1,0.0,0.0,20.0,20.0)\n");
    printf(" -s: silent mode. It does not print anything and uses defaults \n");
    printf(" -v: verbose mode. It prints a lot more information \n");
    printf("\n");
    printf("If no options are given, it prints a help message.\n");
    printf("\n");
    printf("Examples:\n");
    printf("\n");
    printf("   synthimage -p 512 -c 1.0 -m gauss -d 1,0.,0.,10.0,2.0 image.fits \n");
    printf("\n");
    printf("Creates a synthetic image with 512 pixels along its side, with each pixel\n");
    printf("having physical dimension of 1 microarcsec. The image is created from a \n");
    printf("gaussian model with one gaussian component, centered at (0.0,0.0) microarcsec\n");
    printf("from the center of the image and with standard deviation equal to 10.0 and\n");
    printf("2.0 microarcsec along the x- and y- orientations.\n");
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

@param *Npixel an intereger returning the number of pixels per dimension

@param *pixelSize a double returning the size of each pixel in microarcsec

@param *model a string returning the model name

@param *modelNumber an integer returing a number assigned to the particular model for fast referencing

@param *paramstring a string of parameters

@param param[] returns the array of model parameters

\return Returns zero if successful, 1 if not

*/
int verboseinput(char *outFileName, int *Npixel, double *pixelSize, char *model, int *modelNumber, char *paramstring, double param[])
{
  int NpixelInput;               // new value for Npixel
  double pixelSizeInput;         // new value for pixelSize
  int Ncomp;                     // number of components
  double paramInput;             // input value of a parameter
  char line[MAXCHAR];            // line of characters
  char *ptr;                     // pointer used for converting strings to numbers
  int dummyResult;               // dummy variable to test return values of functions
  int iComp,iParam;              // index variables to count model components and parameters
  int index;                     // cummulative index variable to count all model parameters
  
  // ask for new number of pixels
  printf("Number of pixels [%d]: ",*Npixel);
  fgets(line, sizeof line, stdin);
  NpixelInput=strtol(line, &ptr, 10);
  if (NpixelInput!=0)
    *Npixel=NpixelInput;

  // ask for new number of pixels
  printf("Pixel size in microarcsec [%12.5f]: ",*pixelSize);
  fgets(line, sizeof line, stdin);
  pixelSizeInput=strtod(line, &ptr);
  if (pixelSizeInput!=0)
    *pixelSize=pixelSizeInput;

  // ask for new model
  printf("Model [%s]: ",model);
  fgets(line, sizeof line, stdin);
  // do something only if it's not a newline
  if (line[0]!='\n')
    {
      // first remove trailing newline
      line[strcspn(line, "\n")] = 0;
      strcpy(model,line);

      // check model name and assign a number to it for fast future reference
      dummyResult=imageModelCheck(model,modelNumber);
      if (dummyResult!=0)                                    // if there was a problem
	{
	  printErrorSynthimage("model name not recognized\n");
	  return 1;
	}
    }
  
  // ask for new number of model components
  printf("Number of components [%d]: ",(int) param[0]);
  fgets(line, sizeof line, stdin);
  Ncomp=strtol(line, &ptr, 10);
  // if there was a valid input
  if (Ncomp!=0)
    {
      param[0]=Ncomp;
    }
  else  // if not, keep the old number of components
    {
      Ncomp=param[0];
    }

  // cummulative counter of all model parameters
  index=0;
  // ask for new parameters for each component
  for (iComp=0;iComp<=Ncomp-1;iComp++)
    {
      printf("Component \#%d\n",iComp+1);
      {
	for (iParam=0;iParam<=NModelParam(*modelNumber)-1;iParam++)
	  {
	    index++;
	    // start with a small tab
	    printf("   ");
	    // print the description of this model parameter
	    printModelParam(*modelNumber,iParam);
	    // and its current value
	    printf(" [%12.5f]: ",param[index]);
	    fgets(line, sizeof line, stdin);
	    paramInput=strtod(line, &ptr);
	    // if there was a valid parameter entered, update the value
	    if (paramInput!=0)
	      param[index]=paramInput;
	  }
      }
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
		      
  return 0;
}

/*!
\brief Parses the command line for options

\details 
Parses the command line for options. If no options are given,
it prints a help message

The required options are:
- <filename>: sets the input image filename (FITS)

  The optional options are:
  - "-p Npixels": sets the number of image pixels per dimension (default 512)
  - "-c size": physical dimension of each pixel in microarcsec (default 1.0)
  - "-m modelname": the name of the model to be used (default "gauss")
  - "-d param1,param2,...": the values of the various model parameters (separated by commas, with no spaces between them or in quotes) (default 1,0.0,0.0,20.0,20.0)
  - "-s": silent mode. It does not print anything and uses defaults 
  - "-v": verbose mode. It prints a lot more information 

  If no options are given, it prints a help message

\author Dimitrios Psaltis

\version 1.0

\date September 30, 2017

\pre It is called from main()

@param argc an int (as is piped from the unix prompt)

@param argv[] an array of strings (as is piped from the unix prompt)

@param *outFileName a string which provides and returns the output filename

@param *vmode an int with a flag for the chosen verbose mode (0:silent, 1: normal, 2: verbose)

@param *Npixel an intereger returning the number of pixels per dimension

@param *pixelSize a double returning the size of each pixel in microarcsec

@param *model a string returning the model name

@param *modelNumber an integer returing a number assigned to the particular model for fast referencing

@param *paramstring a string of parameters

\return Returns zero if successful, 1 if not

*/
int parse(int argc, char *argv[], char *outFileName, int *vmode, int *Npixel, double *pixelSize, char *model, char *paramstring)
{
  int opt = 0;
  int index;
  char *ptr;                     // pointer used for converting strings to numbers

  opterr=0;            // do not print any other errors

  // initialize default parameters
  *Npixel=NPIXELDEFAULT;
  *pixelSize=PIXELSIZEDEFAULT;
  strcpy(model,MODELDEFAULT);
  strcpy(paramstring,PARAMDEFAULTG);

  if (argc==1)         // if no options are given
    {
      printhelp();     // print help message and return with a code to do nothing
      return 1;
    }

  *vmode=VMODEDEFAULT;                      // default verbose mode "high"

  // parse through arguments with options
  while ((opt = getopt(argc, argv, "svp:c:m:d:")) != -1)
    {
      switch(opt)
	{
	case 'c':                           // size of pixels
	  *pixelSize=strtod(optarg, &ptr);
	  break;
        case 'p':                           // number of pixels
	  *Npixel=strtoumax(optarg, NULL, 10);
	  break;
	case 'm':                           // string with the model name
	  strcpy(model,optarg);
	  break;
	case 'd':                           // string with the various parameters
	  strcpy(paramstring,optarg);
	  break;
	case 's':
	  *vmode=0;                         // verbose mode "silent"
	  break;
	case 'v':
	  *vmode=2;                         // verbose mode "verbose"
	  break;
	case '?':
	  {
	    printErrorSynthimage("Invalid option received\n");
	  }
	  break;
	}
    }
  
  // The variable argc counts the total number of elements in the input,
  //    while the variable optind counts the number of arguments
  //    that have been parsed. If the latter is larger than the
  //    former, then there is no argument after the options

      if (optind >= argc) {
	printErrorSynthimage("Expected argument after options\n");
        return 1;
  }

  if (argc-optind!=1)  // it requires at least one argument with no options
    {
      printErrorSynthimage("Too many arguments\n");
      return 1;
    }

  // output file name is the single non-option argument
  strcpy(outFileName,argv[optind]);

  // check all the required options
  if (*Npixel<=0 || *Npixel>MAXPIXEL) // if the number of pixels is out of range
    {
      printErrorSynthimage("Invalid number of pixels\n");
      return 1;
    }
  if (*pixelSize<=0)                  // if the pixel size is invalid
    {
      printErrorSynthimage("Invalid size of pixels\n");
      return 1;
    }
    
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
  char outFileName[MAXCHAR];                        // string for the output filenames
  char hist[MAXCHAR];                               // string for history in output FITS file
  int vmode;                                        // flag for verbose mode
  
  int Npixel;                                       // size of image along each side
  double pixelSize;                                 // physical sizes of image pixels along the two directions
  double *ImageOut;                                 // pointer to image array

  char model[MAXCHAR];                              // name of the model to be used
  int modelNumber;                                  // integer with number of the model
  char paramstring[MAXCHAR];                        // string with input parameter values
  double param[MAXPARAM];                           // array with parameter values

  int dummyResult;                                  // dummy variable for integer results of functions
  int writeflag;                                    // variable to store result of writing to a file

  // parse the command line
  int parseflag=parse(argc, argv,&outFileName,&vmode,&Npixel, &pixelSize, &model, paramstring);

  // if there was an error in parsing, return with an error code
  if (parseflag!=0) return 1;

  // check model name and assign a number to it for fast future reference
  dummyResult=imageModelCheck(model,&modelNumber);
  if (dummyResult!=0)                                    // if there was a problem
    {
      printErrorSynthimage("model name not recognized\n");
      return 1;
    }

  // set default parameter string based on model
  switch (modelNumber)
    {
    case 0:                                                 // gaussian model 
      strcpy(paramstring,PARAMDEFAULTG);
      break;
    case 1:                                                 // crescent model 
      strcpy(paramstring,PARAMDEFAULTC);
      break;
    }  

  // check the model parameters
  dummyResult=imageParamCheck(modelNumber,paramstring,param);

  // if they are not valud
  if (dummyResult!=0)
    return 1;
  
  // if the verbose mode is selected, check for all parameters
  if (vmode==2)
    {
      dummyResult=verboseinput(outFileName, &Npixel, &pixelSize, model, &modelNumber, paramstring, param);
      // if there was a problem
      if (dummyResult!=0)
	return 1;
    }
  
  // allocate memory for the image array
  ImageOut = (int *)malloc(sizeof(double)*Npixel*Npixel);  // allocate memory to store image

  // calculate the brightness of the image
  switch (modelNumber)
    {
    case 0:                                                 // gaussian model
      dummyResult=gaussModel(Npixel,pixelSize,param,ImageOut);
      break;
    case 1:                                                 // crescent model
      dummyResult=crescentModel(Npixel,pixelSize,param,ImageOut);
      break;
    }

  if (dummyResult!=0)                                    // if there was a problem
    {
      printErrorSynthimage("invalid model parameters\n");
      return 1;
    }
  
  // create a history string to include the model
  strcpy(hist,"synthetic image from model ");
  strcat(hist,model);
  
  writeflag=writeFITSImage(outFileName,Npixel,pixelSize*muarcsecToDegrees,ImageOut,hist);

  if (vmode!=0)
    printf("synthimage: Created a %dx%d synthetic image\n",Npixel,Npixel);
  
  // free the allocated memory
  free(ImageOut);

  if (writeflag==1)
    {
      return 1;
    }
  
  if (vmode!=0)
    printf("synthimage: Wrote synthetic image to file %s\n",outFileName);

  return 0;                                          // normal return
}
