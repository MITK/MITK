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

/**@file
 *    this funtion transposes the image
 */

/** @brief transposes the image, i.e. a permutation of the axes is performed
 *
 *
 *     @param pic          pointer to picture
 *     @param pic_old      pointer to old picture, which is not used in this version
 *     @param permutations_vector  indicates the permutation 
 *     @par Example          3 1 2 
 *     @arg                  third axis will be the first axis
 *     @arg                  first axis will be the second
 *     @arg                  second axis will be the third 
 *  
 *  @return transposed image 
 *
 * $Log$
 * Revision 1.3  2003/01/30 14:30:53  mark
 * in Scale.c _mitkIpFuncScaleBL auskommentiert, wegen Fehler
 *
 * Revision 1.2  2000/05/24 15:29:43  tobiask
 * Changed the installation paths of the package.
 *
 * Revision 1.4  2000/05/04 12:52:11  ivo
 * inserted BSD style license
 *
 * Revision 1.3  2000/03/06 17:02:48  ivo
 * ipFunc now with doxygen documentation.
 *
 * Revision 1.2  2000/02/18 14:58:08  ivo
 * Tags are now copied into newly allocated images.
 * Bugs fixed in mitkIpFuncFrame, mitkIpFuncRegGrow, _mitkIpFuncBorderX and mitkIpFuncHitMiss.
 *
 * Revision 1.1.1.1  1998/07/16  12:04:50  antje
 * initial import
 *
 * Revision 1.1  1995/04/06  12:04:30  uli
 * Initial revision
 */

/*
** ipFunc includefiles
*/ 
#include "mitkIpFuncP.h"
 
mitkIpPicDescriptor *mitkIpFuncTranspose( mitkIpPicDescriptor *pic,
                              mitkIpPicDescriptor *pic_old,
                              int *permutations_vector )
;
#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncTranspose\tDKFZ (Dept. MBI) $Revision$ "__DATE__ };
#endif 
 



/* 
**  the action starts here
*/
mitkIpPicDescriptor *mitkIpFuncTranspose( mitkIpPicDescriptor *pic,
                              mitkIpPicDescriptor *pic_old,
                              int *permutations_vector )

{
  mitkIpPicDescriptor *pic_return;
  mitkIpUInt4_t size  [_mitkIpPicNDIM];
  mitkIpUInt4_t r_size[_mitkIpPicNDIM];
  mitkIpUInt4_t index [_mitkIpPicNDIM];
  mitkIpUInt4_t **r_index;
  mitkIpUInt4_t i;
  mitkIpUInt4_t r_offset;
  int default_perm = 0;


  /*
  **
  */
  if( !pic ) return( NULL );
  if ( pic->dim < 1 || pic->dim > _mitkIpPicNDIM)
    return (NULL);


  /*
  ** create and initialize help vectors 
  */

  /*
  ** permutations_vector == NULL
  ** default permutation (1234 -> 4321)
  */
  if( permutations_vector == NULL )
    {
      permutations_vector = (int *) malloc( 
                                     pic->dim * sizeof( int ));
      if (!permutations_vector)
        return (0);

      for(i=0; i<pic->dim; i++ )
        permutations_vector[i] =  pic->dim - i;
    }
  else
    {

      int tmp[8];
      int error = 0;
      for (i = 0; i < pic->dim; i++)
        tmp[i] = 0;
      for (i = 0; error == 0 && i < pic->dim; i++)
        {
          if( permutations_vector[i] > 0 )
            tmp[permutations_vector[i] - 1]++;
          else
              error = 1;
        }
      for (i = 0; error == 0 && i < pic->dim; i++)
        if (tmp[i] != 1)
            error = 1;
      if (error == 1)
        return (0);
    }


  /* 
  ** take over image header structure and allocate memory 
  */
  mitkIpPicFree( pic_old );
  pic_return = mitkIpPicCopyHeader( pic, NULL );
  if (!pic_return)
    {
      if (default_perm)
        free ((void *) permutations_vector);
      return (0);
    }

  pic_return->data = malloc( _mitkIpPicSize(pic_return) );
  if (!pic_return->data)
    {
      if (default_perm)
        free ((void *) permutations_vector);
      mitkIpPicFree (pic_return);
      return (0);
    }



  /*
  ** fill new dimension to pic_return
  */
  for( i=0; i<pic->dim; i++ ) 
    pic_return->n[i] = pic->n[ ( permutations_vector[i] - 1 ) ];

  /*
  ** fill size vectors for multiplication
  */
  for( i=0; i<_mitkIpPicNDIM; i++)
    {
      size[i] = 0;
      r_size[i] = 0;
    }
  r_size[0] = 1;
  size[0] = 1;
  for( i=1; i<pic->dim; i++ ) 
    {
      size[i] = size[i-1] * pic->n[i-1];
      r_size[i] = r_size[i-1] * pic_return->n[i-1];
    }


  /*
  ** index vectors
  */
  r_index = (mitkIpUInt4_t **) malloc( _mitkIpPicNDIM * sizeof( mitkIpUInt4_t * ) );
  for( i=0; i<_mitkIpPicNDIM; i++)
    r_index[i] = &( index[i] );
  for( i=0; i<pic->dim; i++ ) 
    r_index[i] = &( index[ ( permutations_vector[i] - 1 ) ]);
    

  /*
  ** Makro for all for-loops and switches for all dimensions (1-8)
  ** FOR ALL: dimensions, indizes, data types
  */
  mitkIpPicFORALL_4( mitkIpFuncFORALL, pic, pic_return, index, r_offset, 
            for( r_offset = *(r_index[0]), i=1; i<pic->dim; i++  )
              r_offset += *(r_index[i]) * r_size[i];
          )



 
  free( (void *) r_index );
  if (default_perm) free ((void *) permutations_vector);


  /* Copy Tags */

  mitkIpFuncCopyTags(pic_return, pic);

  return( pic_return );
}
#endif
