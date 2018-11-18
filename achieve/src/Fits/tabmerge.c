/*! \file
\brief 
Merges 2 tables by appending all the rows from the 1st table onto the
2nd table. 

\details
Use: tabmerge intable[ext][filters] outtable[ext]

Merge 2 tables by appending all the rows from the 1st table onto the
2nd table. The 2 tables must have identical structure, with the same
number of columns with the same datatypes.

Examples: 
-  tabmerge in.fit+1 out.fit+2
   append the rows from the 1st extension of the input file
      into the table in the 2nd extension of the output file.
-  tabmerge 'in.fit+1[[PI > 45]' out.fit+2
   Same as the 1st example, except only rows that have a PI
      column value > 45 will be merged into the output table.

In this program, a series of 'if' statements are used to verify that
the input files exist and that they point to valid tables with
identical sets of columns. If all these checks are satisfied
correctly, then additional empty rows are appended to the end of the
output table, and the rows are copied one by one from the input table
to the output table. Since the tables have identical structures, it is
possible here to use the low-level CFITSIO routines that read and
write each row of the table as a raw string of bytes. This is much
faster than having to read and write the numerical values in each
column individually.

Note that in this example the output file is modified in place, rather
than creating a whole new output file. It is often easier to modify
the existing file in this way rather than create a new one, but there
is a slight risk that the output file could be corrupted if the
program crashes before the modifications have been completed (e.g.,
due to a power failure, or the user exits the program with Cntl-C, or
there is insufficient disk space to write the modified file).

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
    int status = 0;   /* CFITSIO status value MUST be initialized to zero! */
    int icol, incols, outcols, intype, outtype, hdunum, check = 1;
    long inrep, outrep, width, inrows, outrows, ii, jj;
    unsigned char *buffer = 0;

    if (argc != 3) { 
      printf("Usage:  tabmerge infile1[ext][filter] outfile[ext]\n");
      printf("\n");
      printf("Merge 2 tables by copying all the rows from the 1st table\n");
      printf("into the 2nd table.  The  2 tables must have identical\n");
      printf("structure, with the same number of columns with the same\n");
      printf("datatypes.  This program modifies the output file in place,\n");
      printf("rather than creating a whole new output file.\n");
      printf("\n");
      printf("Examples: \n");
      printf("\n");
      printf("1. tabmerge intab.fit+1 outtab.fit+2\n");
      printf("\n");
      printf("    merge the table in the 1st extension of intab.fit with\n");
      printf("    the table in the 2nd extension of outtab.fit.\n");
      printf("\n");
      printf("2. tabmerge 'intab.fit+1[PI > 45]' outab.fits+2\n");
      printf("\n");
      printf("    Same as the 1st example, except only rows that have a PI\n");
      printf("    column value > 45 will be merged into the output table.\n");
      printf("\n");
      return(0);
    }

    /* open both input and output files and perform validity checks */
    if ( fits_open_file(&infptr,  argv[1], READONLY,  &status) ||
         fits_open_file(&outfptr, argv[2], READWRITE, &status) )
        printf(" Couldn't open both files\n");
        
    else if ( fits_get_hdu_type(infptr,  &intype,  &status) ||
              fits_get_hdu_type(outfptr, &outtype, &status) )
        printf("couldn't get the type of HDU for the files\n");

    else if (intype == IMAGE_HDU)
        printf("The input HDU is an image, not a table\n");

    else if (outtype == IMAGE_HDU)
        printf("The output HDU is an image, not a table\n");

    else if (outtype != intype)
        printf("Input and output HDUs are not the same type of table.\n");

    else if ( fits_get_num_cols(infptr,  &incols,  &status) ||
              fits_get_num_cols(outfptr, &outcols, &status) )
        printf("Couldn't get number of columns in the tables\n");

    else if ( incols != outcols )
        printf("Input and output HDUs don't have same # of columns.\n");        

    else if ( fits_read_key(infptr, TLONG, "NAXIS1", &width, NULL, &status) )
        printf("Couldn't get width of input table\n");

    else if (!(buffer = (unsigned char *) malloc(width)) )
        printf("memory allocation error\n");

    else if ( fits_get_num_rows(infptr,  &inrows,  &status) ||
              fits_get_num_rows(outfptr, &outrows, &status) )
        printf("Couldn't get the number of rows in the tables\n");

    else  {
        /* check that the corresponding columns have the same datatypes */
        for (icol = 1; icol <= incols; icol++) {
            fits_get_coltype(infptr,  icol, &intype,  &inrep,  NULL, &status);
            fits_get_coltype(outfptr, icol, &outtype, &outrep, NULL, &status);
            if (intype != outtype || inrep != outrep) {
                printf("Column %d is not the same in both tables\n", icol);
                check = 0;
            }
        }

        if (check && !status) 
        {
            /* insert 'inrows' empty rows at the end of the output table */
            fits_insert_rows(outfptr, outrows, inrows, &status);

            for (ii = 1, jj = outrows +1; ii <= inrows; ii++, jj++)
            {
                /* read row from input and write it to the output table */
                fits_read_tblbytes( infptr,  ii, 1, width, buffer, &status);
                fits_write_tblbytes(outfptr, jj, 1, width, buffer, &status);
                if (status)break;  /* jump out of loop if error occurred */
            }

            /* all done; now free memory and close files */
            fits_close_file(outfptr, &status);
            fits_close_file(infptr,  &status); 
        }
    }

    if (buffer)
        free(buffer);

    if (status) fits_report_error(stderr, status); /* print any error message */
    return(status);
}

