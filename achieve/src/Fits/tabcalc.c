/*! \file
\brief 
Computes new values for the specified table column using the input
arithmetic expression.

\details
Use: tabcalc intable expression colname outtable

Computes new values for the specified table column using the input
arithmetic expression, which may be a function of the values in other
table columns or keyword values. The input file is first copied to the
output file, then the output file is updated with the new column
values. If the column doesn't already exist, then a new column will be
appended to the table. Instead of supplying the arithmetic expression
directly on the command line, you can also put the expression in an
ASCII text file, and supply the name of the text file prepended with
the '@' character.

The following arithmetic operators and functions may be used in the expression:

    "addition"         +          "subtraction"        -
    "multiplication"   *          "division"           /
    "negation"         -          "exponentiation"     **   ^
    "absolute value"   abs(x)     "cosine"             cos(x)
    "sine"             sin(x)     "tangent"            tan(x)
    "arc cosine"       arccos(x)  "arc sine"           arcsin(x)
    "arc tangent"      arctan(x)  "arc tangent"        arctan2(x,y)
    "exponential"      exp(x)     "square root"        sqrt(x)
    "natural log"      log(x)     "common log"         log10(x)
    "modulus"          i % j      "random # [0.0,1.0)" random()
    "minimum"          min(x,y)   "maximum"            max(x,y)
    "if-then-else"     b?x:y

Note that the fitscopy program can perform the same operations as tabcalc, using a slightly different command line syntax.

Examples: 

-  tabcalc in.fit+1 'X+0.5' Xc out.fit - calculate new Xc column

  fitscopy 'in.fit+1[col Xc=X+0.5]' out.fit - same as above

-  tabcalc in.fit+1 @expr.txt Xc out.fit - read expression from file

Other example expressions:

  'counts/#exposure' - divide counts column by EXPOSURE keyword

  'counts > 0 ? counts/#exposure : -99' - if counts column is
  greater than 0 then divide counts by the exposure keyword,
  otherwise set the value equal to -99.

  '(counts{-1} * 0.25 + counts * 0.5 + counts{+1} * 0.25) / 3.' 
   compute weighted running mean of counts by averaging the
     values in the previous row, the current row, and the next
     row.  The row offset is enclosed in curly brackets.


Version 1.0: September 29, 2017 (DP)


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
    fitsfile *infptr, *outfptr;  /* FITS file pointers */
    int status = 0;   /*  CFITSIO status value MUST be initialized to zero!  */
    int hdutype, hdunum, ii;

    if (argc != 5) { 
      printf("Usage:  tabcalc infile expression colname outfile\n");
      printf("\n");
      printf("Compute new values for the specified table column using the\n");
      printf("input arithmetic expression which may be a function of the \n");
      printf("values in other table columns. The input file is first copied\n");
      printf("to the output file, then the output file is updated with the\n");
      printf("new column values.  If the column doesn't already exist,\n");
      printf("then a new column will be appended to the table.\n");
      printf("\n");
      printf("Example: \n");
      printf("1. tabcalc intab.fits+1 'counts/#exposure' rate outab.fits\n");
      printf("\n");
      printf("    Calculate the 'rate' column from the values in the\n");
      printf("    'counts' column and the 'exposure' keyword.\n");
      printf("\n");
      printf("2. tabcalc intab.fits+1 'sqrt(X**2 + Y**2)' Radius outab.fits\n");
      printf("\n");
      printf("    Calculate the 'Radius' column from the 'X' and 'Y' cols.\n");
      printf("\n");
      printf("3. tabcalc intab.fits+1 '(rate{-1}+rate+rate{+1})/3' rate3 outab.fits\n");
      printf("\n");
      printf("    Calculate the running mean of the rate column by \n");
      printf("    averaging the values in the previous row, the current\n");
      printf("    row, and the next row\n");
      return(0);
    }
    if (!fits_open_file(&infptr, argv[1], READONLY, &status) )
    {
        if (fits_get_hdu_type(infptr, &hdutype,&status) || 
            hdutype==IMAGE_HDU) {
            printf("Error: input HDU is not a table\n");
        } else {

            fits_get_hdu_num(infptr, &hdunum);  /* save current HDU location */

            if (!fits_create_file(&outfptr, argv[4], &status) )
            {
               /* copy all the HDUs from the input file to the output file */
               for (ii = 1; !status; ii++) {
                  fits_movabs_hdu(infptr, ii, NULL, &status);
                  fits_copy_hdu(infptr, outfptr, 0, &status);
               }

               if (status == END_OF_FILE)
                  status = 0;  /* reset expected error */

               /* move back to initial position in the file */
               fits_movabs_hdu(outfptr, hdunum, NULL, &status); 

               /* argv[2] is the expression, and argv[3] is the col name */
               fits_calculator(outfptr, argv[2], outfptr, argv[3],
                               NULL, &status);

               fits_close_file(outfptr, &status);  /* Done */
            }
        }
        fits_close_file(infptr, &status); 
    } 

    if (status) fits_report_error(stderr, status); /* print any error message */
    return(status);
}

