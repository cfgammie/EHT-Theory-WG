/*!  \file

\brief 
Lists the header keywords in the specified HDU (Header Data Unit) of a file.

\details
Use: listhead infile[ext]

This program will list the header keywords in the specified HDU
(Header Data Unit) of a file. If a HDU name or number is not appended
to the input root file name, then the program will list the keywords
in every HDU in the file.

EXAMPLES;

- listhead file.fit        - list all the headers in the file
- listhead 'file.fit[0]'   - list the primary array header
- listhead 'file.fit[2]'   - list the header of 2nd extension
- listhead file.fit+2      - same as above
- listhead 'file.fit[GTI]' - list header of the 'GTI' extension

This program is very short. It simply opens the input file and then
reads and prints out every keyword in the current HDU in sequence. If
a specific HDU was not specified as part of the input file name, it
then trys to move to the next HDU in the file and list its
keywords. This continues until it reaches the end of the file.

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
    fitsfile *fptr;         /* FITS file pointer, defined in fitsio.h */
    char card[FLEN_CARD];   /* Standard string lengths defined in fitsio.h */
    int status = 0;   /* CFITSIO status value MUST be initialized to zero! */
    int single = 0, hdupos, nkeys, ii;

    if (argc != 2) {
      printf("Usage:  listhead filename[ext] \n");
      printf("\n");
      printf("List the FITS header keywords in a single extension, or, if \n");
      printf("ext is not given, list the keywords in all the extensions. \n");
      printf("\n");
      printf("Examples: \n");
      printf("   listhead file.fits      - list every header in the file \n");
      printf("   listhead file.fits[0]   - list primary array header \n");
      printf("   listhead file.fits[2]   - list header of 2nd extension \n");
      printf("   listhead file.fits+2    - same as above \n");
      printf("   listhead file.fits[GTI] - list header of GTI extension\n");
      printf("\n");
      printf("Note that it may be necessary to enclose the input file\n");
      printf("name in single quote characters on the Unix command line.\n");
      return(0);
    }

    if (!fits_open_file(&fptr, argv[1], READONLY, &status))
    {
      fits_get_hdu_num(fptr, &hdupos);  /* Get the current HDU position */

      /* List only a single header if a specific extension was given */ 
      if (hdupos != 1 || strchr(argv[1], '[')) single = 1;

      for (; !status; hdupos++)  /* Main loop through each extension */
      {
        fits_get_hdrspace(fptr, &nkeys, NULL, &status); /* get # of keywords */

        printf("Header listing for HDU #%d:\n", hdupos);

        for (ii = 1; ii <= nkeys; ii++) { /* Read and print each keywords */

           if (fits_read_record(fptr, ii, card, &status))break;
           printf("%s\n", card);
        }
        printf("END\n\n");  /* terminate listing with END */

        if (single) break;  /* quit if only listing a single header */

        fits_movrel_hdu(fptr, 1, NULL, &status);  /* try to move to next HDU */
      }

      if (status == END_OF_FILE)  status = 0; /* Reset after normal error */

      fits_close_file(fptr, &status);
    }

    if (status) fits_report_error(stderr, status); /* print any error message */
    return(status);
}

