#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
/*! \file
  \brief 
  Analytic models for images.

  \details
  A set of functions to calculate the image brightness of a number of
  analytic models. 

  In this version, it contains 2 analytic models:

  - gauss
  
  A multi-gaussian model. 
  \f[
  I(x,y)=\sum_{i=1}^{N}\frac{F_i}{\sqrt{2\pi \sigma_{x,i}^2\sigma_{y,i}^2}}
  \exp\left[-\frac{\left(x^\prime \sin\theta_i+y^\prime \cos\theta_i\right)^2}{\sigma_{x,i}^2}
            -\frac{\left(x^\prime \cos\theta_i-y^\prime \sin\theta_i\right)^2}{\sigma_{y,i}^2}
      \right]
  \f]
  where \f$x^\prime=x-x_{0,i}\f$ and \f$y^\prime=y-y_{0,i}\f$.

  The first model parameter is the number of Gaussian components.

  The second to seventh model paramerts are the total flux \f$F_i\f$, the coordinates \f$x_{0,i}\f$ 
  and \f$y_{0,1}\f$ of its center, the dispersions \f$\sigma_{x,i}\f$ and \f$\sigma_{y,i}\f$
  along the major and minor axes for the first component, and the orientation of the major
  axis \f$\theta_i\f$ in radians E of N (i.e., measured from the vertical axis).

  If more than one components are present, the same list of parameters is repeated for each
  model.

  There are no requirements for the model parameters. In principle, the total flux may be
  a negative number to allow for modeling subtracted components.

  Because of the limit of 20 total model parameters, up to 3 gaussian components can be accommodated.

  - crescent

  The crescent model of <a href="http://adsabs.harvard.edu/abs/2013MNRAS.434..765K">Kmaruddin & Dexter</a>, 2013, MNRAS 414, 765.
  
   The functional form of the model corresponds to the subtraction of two unit disks.

    The first model parameter is the number of crescent components.

    The second to seventh model paramerts are the total flux \f$F_i\f$, the coordinates \f$x_{0,i}\f$ 
    and \f$y_{0,i}\f$ of its center, the overall size \f$R\f$ of the crescent, its relative thickenss 
    \f$\psi\f$, the degree of symmetry \f$\tau\f$ and its orientation \f$\phi\f$.

    Using this parameters, the crescent is calculated by subtracting two unit desks. The outer disk
    is centered at \f$(x_{0,i},y_{0,i})\f$ and has a radius of \f$R_p=R\f$. The inner disk is centered
    at \f$(x_{0,i}+a,y_{0,i}+b)\f$, where
    \f[
       a=R(1-\tau)\psi \sin\phi
    \f]
    and
    \f[
       b=R(1-\tau)\psi \cos\phi
    \f]
    and has a radius of 
    \f[
       R_n=R(1-\psi)\;.
    \f]

    The brightness of each pixel that is "on" is
    \f[
       V_0=\frac{F_i}{\pi R^2 \psi(2-\psi)}\;.
    \f]

    If more than one components are present, the same list of parameters is repeated for each
    model.

    The requirements on the parameters are: \f$F>0\f$, \f$0<\psi\le 1\f$, \f$0\le\tau<1\f$.


  \author Dimitrios Psaltis
  
  \version 1.0
  
  \date November 14, 2017

  \bug No known bugs
  
  \warning No known warnings
  
  \todo Nothing left

*/
#define MAXCHAR 80                         //!< maximum number of characters for strings
#define MAXPARAM 20                        //!< maximum number of model parameters

#define NMODELS 2                          //!< number of analytic models

#define RED "\x1B[31m"                     //!< color RED for error output
#define RESETCOLOR "\x1B[0m"               //!< color to reset to normal

char modelNames[NMODELS][MAXCHAR]={"gauss","crescent"}; //!< names of known analytic models

int modelsNParam[NMODELS]={6,7};           //!< number of required parameters for each component of each model

/*!
\brief 
   Checks for validity of input model

\details 
   Given a model name (in "model") it checks to see if the model is valid.

   For future reference, it returns an integer modelNumber for that model, with the
   first model being modelNumber=0.

   The number of known models is stored in the constant NMODELS.
   The names of the models are stored in the global array modelNames[NMODELS].
   The integer that corresponds to a particular model is the index of that model
   name in the array modelNames;

\author Dimitrios Psaltis

\version 1.0

\date November 14, 2017

@param model a string with the model name
@param modelNumber an int that, on returrn, gives the integer model number

\return 
   0 if everything was ok; 1 if there was a problem

*/
int imageModelCheck(char *model, int *modelNumber)
{
  *modelNumber=0;            // start searching from first model
  // as long as there are models, check to see if the names match
  while (*modelNumber<NMODELS && strcmp(model,modelNames[*modelNumber])) 
    {
      *modelNumber+=1;
    }

  if (*modelNumber<NMODELS)
    {
      return 0;                  // if all is OK, return 0
    }
  else
    {
      return 1;                  // there was a problem
    }
}

/*!
\brief 
   Checks for validity of input model parameters

\details 
   Given a modelnumber (in "modelNumber") and a string of parameters (in "paramstring"),
   it checks to see if the number and type of parameters are valid.

   Normally, the modelNumber is first identified from a string with the model name in 
   imageModelCheck();

   For future reference, it returns a double array of the parameters in param[]

\author Dimitrios Psaltis

\version 1.0

\date November 14, 2017

@param modelNumber an int with the number of the model to be used
@param paramstring[] a string with the parameter string of the model
@param param[] a double array with the parameters of the model

\return 
   0 if everything was ok; 1 if there was a problem

*/
int imageParamCheck(int modelNumber, char *paramstring, double param[])
{

  char *ptrNext;           // pointer for the next segment of the string
  char *token;             // individual tokens in the parameter string
  int Ncomp;               // number of model components
  int index;               // counting index
  
  // split parameter string into tokens separated by commas

  // first token is the number of components
  token=strtok(paramstring,",");
  Ncomp=strtol(token,&ptrNext,10);

  // if the number of components is invalid
  if (Ncomp==0)
    {
      // print an error message and return with an error code
      fprintf(stderr,RED "Invalid number of model components\n" RESETCOLOR);
      return 1;
    }
  // number of components it the first parameter in the array
  param[0]=Ncomp;
  index=0;
  while (token!=NULL)
    {
      token=strtok(NULL,",");
      if (token!=NULL)
	{
	  index++;
	  param[index] = strtod(token,&ptrNext);
	}
    }
  // check to see if enough parameters were given
  if (index!=Ncomp*modelsNParam[modelNumber])
    {
      // print an error message and return with an error code
      fprintf(stderr,RED "Invalid number of model parameters\n" RESETCOLOR);
      return 1;
    }

  return 0;
}

/*!
\brief 
   Prints the description of a given parameter of a given model

\details 
   Given a modelnumber (in "modelNumber") and a parameter number (in "paramNumber"),
   it prints the description of that paramNumber, usually used in verbose inputs.

\author Dimitrios Psaltis

\version 1.0

\date November 16, 2017

@param modelNumber an int with the number of the model to be used (starting at 0)
@param paramNumber an int with the number of the parameter to be printed (starting at 0)

\return 
   Nothing

*/
void printModelParam(int modelNumber, int paramNumber)
{

  switch(modelNumber)
    {
    case 0:                 // gaussian model
      switch(paramNumber)
	{
	case 0:
	  printf("Total Flux");
	  break;
	case 1:
	  printf("x-location of center (x_0)");
	  break;
	case 2:
	  printf("y-location of center (y_0)");
	  break;
	case 3:
	  printf("Dispersion along major axis (sigma_x)");
	  break;
	case 4:
	  printf("Dispersion along minor axis (sigma_y)");
	  break;
	case 5:
	  printf("Orientation of major axis in degrees E of N (theta)");
	  break;
	}
      break;
    case 1:                 // crescent model
      switch(paramNumber)
	{
	case 0:
	  printf("Total Flux");
	  break;
	case 1:
	  printf("x-location of center (x_0)");
	  break;
	case 2:
	  printf("y-location of center (y_0)");
	  break;
	case 3:
	  printf("Overall size of the crescent (R)");
	  break;
	case 4:
	  printf("Relative thickness (0<psi<=1)");
	  break;
	case 5:
	  printf("Relative asymmetry (0<=tau<1)");
	  break;
	case 6:
	  printf("Relative orientation (phi)");
	  break;
	}
    }
  
  return;
}

/*!
\brief 
   Prints the description of a given parameter of a given model

\details 
   Given a modelnumber (in "modelNumber"), it returns the number of model parameters
   that are required

\author Dimitrios Psaltis

\version 1.0

\date November 16, 2017

@param modelNumber an int with the number of the model to be used (starting at 0)

\return 
   an int with the number of model parameters required by this model

*/
int NModelParam(int modelNumber)
{
  // just reads it from the global array
  return modelsNParam[modelNumber];
}

/*!
\brief 
   Fills a double array with the brigtness of a multi-Gaussian analytic model

\details 
   Given a number of pixels Npixel, the physical size of each pixel pixelSize and an array of model 
   parameters param[], it fills the Npixel*Npixel double array Image[] with the brightness of a multi
   Gaussian model.

   The functional form of the model is
    \f[
    I(x,y)=\sum_{i=1}^{N}\frac{F_i}{\sqrt{2\pi \sigma_{x,i}^2\sigma_{y,i}^2}}
    \exp\left[-\frac{\left(x^\prime \sin\theta_i+y^\prime \cos\theta_i\right)^2}{\sigma_{x,i}^2}
              -\frac{\left(x^\prime \cos\theta_i-y^\prime \sin\theta_i\right)^2}{\sigma_{y,i}^2}
        \right]
    \f]
    where \f$x^\prime=x-x_{0,i}\f$ and \f$y^\prime=y-y_{0,i}\f$.

    The first model parameter is the number of Gaussian components.

    The second to seventh model paramerts are the total flux \f$F_i\f$, the coordinates \f$x_{0,i}\f$ 
    and \f$y_{0,i}\f$ of its center, the dispersions \f$\sigma_{x,i}\f$ and \f$\sigma_{y,i}\f$
    along the major and minor axes for the first component, and the orientation of the major
    axis \f$\theta_i\f$ in degrees E of N (i.e., measured from the vertical axis).

    If more than one components are present, the same list of parameters is repeated for each
    model.

    There are no requirements for the model parameters. In principle, the total flux may be
    a negative number to allow for modeling subtracted components.

    Because of the limit of 20 total model parameters, up to 3 gaussian components can be accommodated.
\author Dimitrios Psaltis

\version 1.0

\date November 14, 2017

@param Npixel an int with the number of pixels per dimension of the image
@param pixelSize a double with the physical size of the pixel
@param param[] a double array with the parameters of the model
@param *Image a pointer to a double array, which will be filled with the brightness of the
multi-Gaussian image.

\return 
   0 if everything was ok; for now, it always returns zero

*/
int gaussModel(int Npixel, double pixelSize, double param[], double *Image)
{
  int index;                                     // counting index
  int ix,iy;                                     // counting indeces for the x- and y-directions

  // initialize the image
  for (ix=1;ix<=Npixel*Npixel;ix++)
    {
      *(Image+ix-1)=0.0;
    }
  
  // first parameter is the number of model components
  int Ncomp=param[0];
  // cycle through all components
  for (index=1;index<=Ncomp;index++)
    {
      // next parameter is the total flux
      double F=param[(index-1)*modelsNParam[0]+1];
      // next two parameters are the coordinates of the center of the component
      double x0i=param[(index-1)*modelsNParam[0]+2];
      double y0i=param[(index-1)*modelsNParam[0]+3];
      // next two parameters are the dispersions of the component along the major and minor axes
      double sxi=param[(index-1)*modelsNParam[0]+4];
      double syi=param[(index-1)*modelsNParam[0]+5];
      // last parameter is the orientation fo the major axis that needs to be converted from degrees to rad
      double thi=param[(index-1)*modelsNParam[0]+6]*M_PI/180.0;

      // define a rew quantities to speed up calculations
      double invsx2=0.5/(sxi*sxi);
      double invsy2=0.5/(syi*syi);
      double costh=cos(thi);
      double sinth=sin(thi);
      double norm=F/(2.*M_PI*sxi*syi);
	    
      // now add the brightness of each component to the image
      for (ix=1;ix<=Npixel;ix++)
	{
	  // calculate the x-location in physical units centered at the center of the image
	  // note that the x-axis is increasing to the left (East is left)
	  double x=-(ix-Npixel/2)*pixelSize;
	  for (iy=1;iy<=Npixel;iy++)
	    {
	      double y=(iy-Npixel/2)*pixelSize;
	      // calculate the coordinates in the frame rotated along the major and minor axes 
	      double xp=(x-x0i)*sinth+(y-y0i)*costh;
	      double yp=(x-x0i)*costh-(y-y0i)*sinth;
	      // calculate the linear pointer location of this pixel
	      int indexto=indexArr(iy,ix,Npixel,Npixel);
	      *(Image+indexto)+=norm*exp(-invsx2*xp*xp-invsy2*yp*yp);
	    }
	}
      
    }
      
  return 0;                                      // return with everything is OK
  
}

/*!
\brief 
   Fills a double array with the brigtness of an analytic crescent model.

\details 
   Given a number of pixels Npixel, the physical size of each pixel pixelSize and an array of model 
   parameters param[], it fills the Npixel*Npixel double array Image[] with the brightness of the crescent
   mode  of  
   <a href="http://adsabs.harvard.edu/abs/2013MNRAS.434..765K">Kmaruddin & Dexter</a>, 2013, MNRAS 414, 765.

   The functional form of the model corresponds to the subtraction of two unit disks.

    The first model parameter is the number of crescent components.

    The second to seventh model paramerts are the total flux \f$F_i\f$, the coordinates \f$x_{0,i}\f$ 
    and \f$y_{0,i}\f$ of its center, the overall size \f$R\f$ of the crescent, its relative thickenss 
    \f$\psi\f$, the degree of symmetry \f$\tau\f$ and its orientation \f$\phi\f$.

    Using this parameters, the crescent is calculated by subtracting two unit desks. The outer disk
    is centered at \f$(x_{0,i},y_{0,i})\f$ and has a radius of \f$R_p=R\f$. The inner disk is centered
    at \f$(x_{0,i}+a,y_{0,i}+b)\f$, where
    \f[
       a=R(1-\tau)\psi \sin\phi
    \f]
    and
    \f[
       b=R(1-\tau)\psi \cos\phi
    \f]
    and has a radius of 
    \f[
       R_n=R(1-\psi)\;.
    \f]

    The brightness of each pixel that is "on" is
    \f[
       V_0=\frac{F_i}{\pi R^2 \psi(2-\psi)}\;.
    \f]

    If more than one components are present, the same list of parameters is repeated for each
    model.

    The requirements on the parameters are: \f$F>0\f$, \f$0<\psi\le 1\f$, \f$0\le\tau<1\f$.

    Because of the limit of 20 total model parameters, up to 2 crescent components can be accommmodated.

\author Dimitrios Psaltis

\version 1.0

\date November 14, 2017

@param Npixel an int with the number of pixels per dimension of the image
@param pixelSize a double with the physical size of the pixel
@param param[] a double array with the parameters of the model
@param *Image a pointer to a double array, which will be filled with the brightness of the
multi-Gaussian image.

\return 
   0 if everything was ok; the number of components with invalid parameters, if there were any

*/
int crescentModel(int Npixel, double pixelSize, double param[], double *Image)
{
  int index;                                     // counting index
  int ix,iy;                                     // counting indeces for the x- and y-directions
  int result=0;                                  // count number of problems with parameter values

  // initialize the image
  for (ix=1;ix<=Npixel*Npixel;ix++)
    {
      *(Image+ix-1)=0.0;
    }
  
  // first parameter is the number of model components
  int Ncomp=param[0];
  // cycle through all components
  for (index=1;index<=Ncomp;index++)
    {
      // next parameter is the total flux
      double F=param[(index-1)*modelsNParam[0]+1];
      // next two parameters are the coordinates of the center of the component
      double x0i=param[(index-1)*modelsNParam[0]+2];
      double y0i=param[(index-1)*modelsNParam[0]+3];
      // next parameter is the overall size of the crescent
      double R=param[(index-1)*modelsNParam[0]+4];
      // next parameter is the relative thickness
      double psi=param[(index-1)*modelsNParam[0]+5];
      // next parameter is the asymmetry
      double tau=param[(index-1)*modelsNParam[0]+6];
      // final parameter is orientation
      double phi=param[(index-1)*modelsNParam[0]+7];

      // check if the parameters are valid
      if (F>0.0 && R>0 && psi>0.0 && psi<=1.0 && tau>=0 && tau<1)
	{
	  // define a rew quantities to speed up calculations
	  double V0=F/(M_PI*R*R*psi*(2.0-psi));       // brightness of each "on" pixel
	  double Rp=R;                                // radius of outer unit disk
	  double Rn=R*(1.0-psi);                      // radius of inner unit disk
	  double a=R*(1.0-tau)*psi*sin(phi);          // horizontal displacement of inner disk
	  double b=R*(1.0-tau)*psi*cos(phi);          // vertical displacement of inner disk
	  
	  // now add the brightness of each component to the image
	  for (ix=1;ix<=Npixel;ix++)
	    {
	      // calculate the x-location in physical units centered at the center of the image
	      // note that the x-axis is increasing to the left (East is left)
	      double x=-(ix-Npixel/2)*pixelSize;
	      for (iy=1;iy<=Npixel;iy++)
		{
		  double y=(iy-Npixel/2)*pixelSize;
		  // calculate the linear pointer location of this pixel
		  int indexto=indexArr(iy,ix,Npixel,Npixel);

		  double rout=sqrt((x-x0i)*(x-x0i)+(y-y0i)*(y-y0i));          // distance from center of outer disk
		  double rin=sqrt((x-x0i-a)*(x-x0i-a)+(y-y0i-b)*(y-y0i-b));   // distance from center of inner disk
		  // if the point is within the outer disk and outside the inner disk, set its brightnes
		  if (rout<Rp && rin>Rn) 
		      *(Image+indexto)+=V0;
		}
	    }
	}    
      else   // if there was a problem with some parameter, add it to the counter of problems
	{
	  result+=1;
	}
      
    }
      
  return result;                                      // return with number of problems
  
}
