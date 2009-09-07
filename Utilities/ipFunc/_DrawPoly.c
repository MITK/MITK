/*****************************************************************************

 Copyright (c) 1993-2000,  Div. Medical and Biological Informatics, 
 Deutsches Krebsforschungszentrum, Heidelberg, Germany
 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

 - All advertising materials mentioning features or use of this software must 
   display the following acknowledgement: 
          
     "This product includes software developed by the Div. Medical and 
      Biological Informatics, Deutsches Krebsforschungszentrum, Heidelberg, 
      Germany."

 - Neither the name of the Deutsches Krebsforschungszentrum nor the names of 
   its contributors may be used to endorse or promote products derived from 
   this software without specific prior written permission. 

   THIS SOFTWARE IS PROVIDED BY THE DIVISION MEDICAL AND BIOLOGICAL
   INFORMATICS AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE DIVISION MEDICAL AND BIOLOGICAL INFORMATICS,
   THE DEUTSCHES KREBSFORSCHUNGSZENTRUM OR CONTRIBUTORS BE LIABLE FOR 
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

 Send comments and/or bug reports to:
   mbi-software@dkfz-heidelberg.de

*****************************************************************************/

/*
 *
 *---------------------------------------------------------------------
 * DESCRIPTION
 *  this function draws the edge of an ROI to an image
 *
 * FUNCTION DECLARATION
 *  mitkIpPicDescriptor  *_mitkIpFuncDrawPoly( mitkIpPicDescriptor *pic_old,
 *                                     ipUInt4_t       *pol_x,
 *                                     ipUInt4_t       *pol_y,  
 *                                     ipUInt4_t       no_pts, 
 *                                     ipFloat8_t      *a,
 *                                     ipFloat8_t      *b);
 *
 * PARAMETERS
 *  pic_old   - pointer to original image
 *  pol_x     - vector with the x-coordinates of the points which form
 *              form the roi
 *  pol_y     - vector with the y-coordinates of the points which form
 *              form the roi
 *  no_pts    - number of points used to describe ROI
 *  a         -       
 *  b         -
 *
 * REMARKS
 *  this function could just be used for 2D images
 *
 * RETURN VALUES
 *  pic_help  -  image with roi                          
 *
 * AUTHOR & DATE
 *  Antje Schroeder	08.11.95
 *
 * UPDATES
 *  a short history of the file
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */
#ifndef lint
  static char *what = { "@(#)_mitkIpFuncDrawPoly\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif

/* include files                                                       */

#include "mitkIpFuncP.h"


/* -------------------------------------------------------------------- */

/*
** mitkIpFuncDrawPoly
*/

mitkIpPicDescriptor *_mitkIpFuncDrawPoly ( mitkIpPicDescriptor *pic_old,
                                   ipUInt4_t       *pol_x,
                                   ipUInt4_t       *pol_y,
                                   ipUInt4_t       no_pts, 
                                   ipFloat8_t      *a,
                                  ipFloat8_t      *b )
{

   mitkIpPicDescriptor *pic_help;
   ipUInt4_t       i, j;
   ipUInt4_t       y;
   ipUInt4_t       beg, end;

   /* allocate memory for pic help                                      */

   pic_help = mitkIpPicCopyHeader ( pic_old, NULL );
   if ( pic_help == NULL )
     {
        _mitkIpFuncSetErrno ( mitkIpFuncPICNEW_ERROR );
        return ( mitkIpFuncERROR );
     }
   pic_help->type = mitkIpPicUInt;
   pic_help->bpe  = 8;
   pic_help->data = calloc ( _mitkIpPicElements ( pic_help ), pic_help->bpe / 8 );
   if ( pic_help->data == NULL )
     {
        mitkIpPicFree ( pic_help );
        _mitkIpFuncSetErrno ( mitkIpFuncMALLOC_ERROR );
        return ( mitkIpFuncERROR );
     }

   /* draw polygon to pic_help                                          */

   for ( i = 0; i < no_pts-1; i++ )
     { 
        if ( pol_x[i] == pol_x[i+1] )
          {
             if ( pol_y[i] < pol_y[i+1] )
               {
                  beg = pol_y[i];
                  end = pol_y[i+1];
               }
             else
               {
                  end = pol_y[i];
                  beg = pol_y[i+1];
               }
             for ( j = beg; j < end; j++ ) 
                (( ipUInt1_t * )pic_help->data )[ j * pic_help->n[1] + pol_x[i]] = 100;
          }
        else 
          {
             if ( pol_x[i] < pol_x[i+1] )
               {
                  beg = pol_x[i];
                  end = pol_x[i+1];
               }
             else
               {
                  end = pol_x[i];
                  beg = pol_x[i+1];
               }
             if ( pol_y[i] == pol_y[i+1] ) end = beg -1;         
 
             for ( j = beg; j <= end; j++ )
               {
                  y = a[i] * j + b[i];
                  (( ipUInt1_t * )pic_help->data )[ y * pic_help->n[1] + j ] = 100;
               }
          }
        
        if ( pol_y[i] == pol_y[i+1] )
          {
             if ( pol_x[i] < pol_x[i+1] )
               {
                  beg = pol_x[i];
                  end = pol_x[i+1];
               }
             else
               {
                  end = pol_x[i];
                  beg = pol_x[i+1];
               }

             for ( j = beg; j < end; j++ ) 
                (( ipUInt1_t * )pic_help->data )[ j + pic_help->n[1] * pol_y[i]] = 100;
          }
        else 
          {
             if ( pol_y[i] < pol_y[i+1] )
               {
                  beg = pol_y[i];
                  end = pol_y[i+1];
               }
             else
               {
                  end = pol_y[i+1];
                  beg = pol_y[i];
               }
             if ( pol_x[i] == pol_x[i+1] ) end = beg - 1;         
 
             for ( j = beg; j <= end; j++ )
               {
                  y =  ( j - b[i] ) / a[i];
                  (( ipUInt1_t * )pic_help->data )[ j * pic_help->n[1] + y ] = 100;
               }
          }
     }

   if ( pol_x[no_pts-1] == pol_x[0] )
     {
        if ( pol_y[no_pts-1] < pol_y[0] )
          {
             beg = pol_y[no_pts-1];
             end = pol_y[0];
          }
        else
          {
             end = pol_y[no_pts-1];
             beg = pol_y[0];
          }

        for ( j = beg; j < end; j++ ) 
           (( ipUInt1_t * )pic_help->data )[ j * pic_help->n[1] + pol_x[no_pts-1]] = 100;
     }
   else 
     {
        if ( pol_x[no_pts-1] < pol_x[0] )
          {
             beg = pol_x[no_pts-1];
             end = pol_x[0];
          }
        else
          {
             end = pol_x[no_pts-1];
             beg = pol_x[0];
          }
        if ( pol_y[no_pts-1] == pol_y[0] ) end = beg -1;         

        for ( j = beg; j <= end; j++ )
          {
             y = a[no_pts-1] * j + b[no_pts-1];
             (( ipUInt1_t * )pic_help->data )[ y * pic_help->n[1] + j ] = 100;
          }
     }
   
   if ( pol_y[no_pts-1] == pol_y[0] )
     {
        if ( pol_x[no_pts-1] < pol_x[0] )
          {
             beg = pol_x[no_pts-1];
             end = pol_x[0];
          }
        else
          {
             end = pol_x[no_pts-1];
             beg = pol_x[0];
          }

        for ( j = beg; j < end; j++ ) 
           (( ipUInt1_t * )pic_help->data )[ j + pic_help->n[1] * pol_y[no_pts-1]] = 100;
     }
   else 
     {
        if ( pol_y[no_pts-1] < pol_y[0] )
          {
             beg = pol_y[no_pts-1];
             end = pol_y[0];
          }
        else
          {
             end = pol_y[no_pts-1];
             beg = pol_y[0];
          }
        if ( pol_x[no_pts-1] == pol_x[0] ) end = beg - 1;         

        for ( j = beg; j <= end; j++ )
          {
             y =  ( j - b[no_pts-1] ) / a[no_pts-1];
             (( ipUInt1_t * )pic_help->data )[ j * pic_help->n[1] + y ] = 100;
          }
     }

   return ( pic_help );
}
