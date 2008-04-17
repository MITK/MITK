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
 *   Rotates image by a given angle 
 */

/**  @brief Rotates image by a given angle.
 *
 *   The angle has to be a multiple of 90 degrees.
 *
 *  @param pic      pointer to picture 
 *  @param pic_old  pointer to picture old, which is not used in this version 
 *  @param grad     angle array for every axes 
 *  @param order    order of the axes 
 *               parameter example:  grad:   90  270  90
 *               order:  3   1    2
 *        means:  first step:   the third axis is rotated with 90 degrees
 *                second step:  the first axis is rotated with 90 degrees
 *                third step:   the second axis is rotated with 270 degrees
 *  
 *  @return image 
 *
 * @author Judith Dilo  @date 5.5.1990 
 * @author Ulrike Guennel @date 18.3.1994 
 *
 *
 *   This a total rewritten version of 'ipRot' written by Judith Dilo
 *    7.6.1990  by J.D.  (small improvements) 
 *   18.3.1994  by U.G.  (new pic format)
 *   $Log$
 *   Revision 1.3  2003/08/21 08:20:06  tobiask
 *   Removed warnings.
 *
 *   Revision 1.2  2000/05/24 15:29:43  tobiask
 *   Changed the installation paths of the package.
 *
 *   Revision 1.4  2000/05/04 12:52:11  ivo
 *   inserted BSD style license
 *
 *   Revision 1.3  2000/03/06 17:02:45  ivo
 *   ipFunc now with doxygen documentation.
 *
 *   Revision 1.2  2000/02/18 14:58:06  ivo
 *   Tags are now copied into newly allocated images.
 *   Bugs fixed in ipFuncFrame, ipFuncRegGrow, _ipFuncBorderX and ipFuncHitMiss.
 *
 * Revision 1.1.1.1  1998/07/16  12:04:49  antje
 * initial import
 *
 * Revision 1.1  1995/03/07  16:42:15  uli
 * Initial revision
 */

/*
** ipFunc includefiles
*/ 
#include "ipFuncP.h"

ipPicDescriptor *ipFuncRotate( ipPicDescriptor *pic,
                               ipPicDescriptor *pic_old,
                               int *grad, int *order );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)ipFuncRotate\tDKFZ (Dept. MBI) $Revision$ "__DATE__ };
#endif 
 
 
/*
** private macros
*/
#define COEFF_DEF( x,i ) (( (x) > (0)) ? ((x) - *p_index[i]) : (*p_index[i]) )
 

/* 
**  the action starts here
*/
ipPicDescriptor *ipFuncRotate( ipPicDescriptor *pic,
                               ipPicDescriptor *pic_old,
                               int *grad, int *order )

{
  ipPicDescriptor *pic_return;
  ipUInt4_t *p_index[_mitkIpPicNDIM];
  ipUInt4_t index[_mitkIpPicNDIM];
  ipUInt4_t coeff_sum[_mitkIpPicNDIM];
  ipUInt1_t coefficient[_mitkIpPicNDIM]; 
  ipUInt4_t r_size[_mitkIpPicNDIM];
  ipUInt4_t r_offset,i;
  int  order_alloc = 0;

  /*
  **
  */
  if( !pic ) return( NULL );
  if( pic->dim < 2 || pic->dim > _mitkIpPicNDIM ) return( NULL );

  /*  take over image header structure and allocate memory */
  ipPicFree( pic_old );
  pic_return = ipPicCopyHeader( pic, NULL );
  if( !pic_return ) return( NULL );
  pic_return->data = malloc( _ipPicSize(pic_return) );
  if( !pic_return->data )
    {
      ipPicFree( pic_return );
      return( NULL );
    }

      
  /*
  ** init pic_return->n, pointer to index and calculated_coeff
  ** init coefficient matrix, r_size
  */
  for( i = 0; i< _mitkIpPicNDIM; i++ )
    {
      pic_return->n[i] = pic->n[i];
      p_index[i] = &index[i];
      coeff_sum[i] = 0;
      coefficient[i] = 0;
      r_size[i] = 0;
    }

  /*
  ** init the order array if NULL
  */
  if( !order )
    {
      order = (int *) malloc( pic->dim * sizeof( int ) );
      if( !order )
        {
          ipPicFree( pic_return );
          return( NULL );
        }
      order_alloc = 1;
      for( i=0; i<_mitkIpPicNDIM; i++ )
          order[i] = i+1;
    }
  else
    {
      /*
      ** test if order correct
      */
      int tt[_mitkIpPicNDIM];
      for( i=0; i<_mitkIpPicNDIM; i++ ) tt[i] = 0;
      for( i=0; i< pic->dim; i++ ) 
        { 
          if( order[i] > 0 && order[i]<= (ipInt4_t) pic->dim )
            tt[ order[i]-1 ]++;
          else
            i = _mitkIpPicNDIM + 1;
        }
      if( i == pic->dim )
        {
          for( i=0; i< pic->dim; i++ )
            {
              if( tt[i] != 1 )
                i = _mitkIpPicNDIM + 1;
            }
        }
      if( i > _mitkIpPicNDIM  )
        {
          ipPicFree( pic_return );
          return( NULL );
        }
    }
  
  /*
  ** fill of the coefficient array and change the p_index array
  */
  for( i = 0; i<pic->dim; i++ )
    {
      int d1,d2;
      ipUInt4_t *tmp;

      d1 = order [i]-1; 
      d2 = (d1+1 >= (ipInt4_t) pic->dim ) ? ( 0 ) : ( d1+1 ); 

      switch( grad[d1]  )
        {
          case 0:
                   break;
          case 90:
                   tmp = p_index[d2];
                   p_index[d2] = p_index[d1];
                   p_index[d1] = tmp;
                   
                   coefficient[ p_index[d2] - index ] =
                      (coefficient[ p_index[d2] - index ] == 1 ) ? 0 : 1;
                   break;
          case 180:
                   coefficient[ p_index[d1] - index ] =
                      (coefficient[ p_index[d1] - index ] == 1 ) ? 0 : 1;
                   coefficient[ p_index[d2] - index ] =
                      (coefficient[ p_index[d2] - index ] == 1 ) ? 0 : 1;
                   break;
          case 270:
                   tmp = p_index[d2];
                   p_index[d2] = p_index[d1];
                   p_index[d1] = tmp;
                   
                   coefficient[ p_index[d1] - index ] =
                      (coefficient[ p_index[d1] - index ] == 1 ) ? 0 : 1;
                   break;
          default:
                   {
                     if( order_alloc == 1 )
                       free( (void *) order );
                     ipPicFree( pic_return );
                     return( NULL );
                     break;
                   }
        }
    }
 

  /*
  ** calculate coefficient, and pic_return->n[i] 
  */
  for( i=0; i<pic->dim; i++  )
    {
      coeff_sum[ i ] = (pic->n[p_index[i] - index]-1) 
                       * coefficient[ p_index[i] - index];
      pic_return->n[i] = pic->n[p_index[i] - index];
    }

  /*
  ** calc multiplier for coefficients
  */
  r_size[0] = 1;
  for( i=1; i<pic->dim; i++ )
    {
      r_size[i] = r_size[i-1] * pic_return->n[i-1];
    }


  /*
  ** Makro for all for-loops and switches for all dimensions (1-8)
  ** FOR ALL: dimensions, indizes, data types
  */
  mitkIpPicFORALL_4( mitkIpFuncFORALL, pic, pic_return, index, r_offset,
            for( r_offset = COEFF_DEF( coeff_sum[0],0) , i=1; i<pic->dim; i++  )
                r_offset += COEFF_DEF( coeff_sum[i],i ) * r_size[i]; 
          )

  /*
  ** free allocated memory
  */
  if( order_alloc == 1 )
    free( (void *) order );

  /* Copy Tags */

  strncpy( pic_return->info->version, pic->info->version, _mitkIpPicTAGLEN );
  pic_return->info->tags_head = _ipPicCloneTags( pic->info->tags_head );
  pic_return->info->write_protect = pic->info->write_protect;
                        
  return( pic_return );
}
#endif
