#include <string.h>
#include <stdio.h>
#include "fitsio.h"

int main(int argc, char *argv[])
{
    fitsfile *infptr, *outfptr;  /* FITS file pointers */
    int status = 0;   /* CFITSIO status value MUST be initialized to zero! */
    int hdutype, hdunum, ii;

    if (argc != 4) { 
      printf("Usage:  tabselect infile expression outfile\n");
      printf("\n");
      printf("Copy selected rows from the input table to the output file\n");
      printf("based on the input boolean expression.  The expression may \n");
      printf("be a function of the values in other table columns or header \n");
      printf("keyword values.  If the expression evaluates to 'true' then \n");
      printf("that row is copied to the output file.\n");
      printf("\n");
      printf("Example: \n");
      printf("1. tabselect intab.fits+1 'counts > 0' outab.fits\n");
      printf("\n");
      printf("    copy rows that have a positive 'counts' column value\n");
      printf("\n");
      printf("2. tabselect intab.fits+1 'gtifilter()' outab.fits\n");
      printf("\n");
      printf("    Select rows which have a Time column value that is\n");
      printf("    within one of the Good Time Intervals (GTI) which are\n");
      printf("    defined in a separate GTI extension in the same file.\n"); 
      printf("\n");
      printf("3. tabselect intab.fits+1 'regfilter(\"pow.reg\")' outab.fits\n");
      printf("\n");
      printf("    Select rows which have X,Y column coordinates located\n");
      printf("    within the spatial region defined in the file named\n");
      printf("    'pow.reg'.  This is an ASCII text file containing a\n");
      printf("    list of one or more geometric regions such as circle,\n");
      printf("    rectangle, annulus, etc.\n");
      return(0);
    }
    if (!fits_open_file(&infptr, argv[1], READONLY, &status) )
    {
        if (fits_get_hdu_type(infptr, &hdutype,&status) || 
            hdutype==IMAGE_HDU) {
            printf("Error: input HDU is not a table\n");
        } else {

            fits_get_hdu_num(infptr, &hdunum);  /* save current HDU location */

            if (!fits_create_file(&outfptr, argv[3], &status) )
            {
               /* copy all the HDUs from the input file to the output file */
               for (ii = 1; !status; ii++) {
                  if ( !fits_movabs_hdu(infptr, ii, NULL, &status) )
                     fits_copy_hdu(infptr, outfptr, 0, &status);
               }

               if (status == END_OF_FILE)
                  status = 0;  /* reset expected error */

               /* move back to initial position in the file */
               fits_movabs_hdu(outfptr, hdunum, NULL, &status); 

               /* argv[2] is the expression */
               /* input and output files are the same, so delete rows that */
               /* do not satisfy the expression */
               fits_select_rows(outfptr, outfptr, argv[2], &status);

               fits_close_file(outfptr, &status);  /* Done */
            }
        }
        fits_close_file(infptr, &status); 
    } 

    if (status) fits_report_error(stderr, status); /* print any error message */
    return(status);
}

