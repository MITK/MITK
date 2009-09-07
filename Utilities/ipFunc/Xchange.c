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
 *  exchange of two image buffers
 */

/** exchange of two image buffers
 *
 *   @param pic1 pointer to pic descriptor
 *   @param pic2 pointer to pic descriptor
 *  
 *
 * @author Ulrike Guennel @date 07.7.1995 
 *
 * $Log$
 * Revision 1.3  2003/08/21 08:44:44  tobiask
 * Removed warnings.
 *
 * Revision 1.2  2000/05/24 15:29:43  tobiask
 * Changed the installation paths of the package.
 *
 * Revision 1.4  2000/05/04 12:52:11  ivo
 * inserted BSD style license
 *
 * Revision 1.3  2000/04/04 11:57:10  ivo
 * no message
 *
 * Revision 1.2  2000/02/18 14:58:09  ivo
 * Tags are now copied into newly allocated images.
 * Bugs fixed in mitkIpFuncFrame, mitkIpFuncRegGrow, _mitkIpFuncBorderX and mitkIpFuncHitMiss.
 *
 * Revision 1.1.1.1  1998/07/16  12:04:50  antje
 * initial import
 *
 * Revision 1.1  1995/03/07  17:01:02  uli
 * Initial revision
 *
 */

 
 /*
** ipFunc includefiles
*/ 
#include "mitkIpFuncP.h"

void mitkIpFuncXchange( mitkIpPicDescriptor **pic1,
                mitkIpPicDescriptor **pic2 );

#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)mitkIpFuncXchange\tDKFZ (Dept. MBI) $Revision$ "__DATE__ };
#endif 
 

/*
** private macros
*/
 

/* 
**  the action starts here
*/
void mitkIpFuncXchange( mitkIpPicDescriptor **pic1,
                mitkIpPicDescriptor **pic2 )

{

  mitkIpPicDescriptor *tmp_pic;
 
  tmp_pic = *pic1;
  *pic1   = *pic2;
  *pic2   = tmp_pic;
 
  return;
}
#endif

