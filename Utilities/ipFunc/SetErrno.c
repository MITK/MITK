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
 *   this functions sets a global error-number (ipFuncErrno) if an error
 *   occured     
 */

/**  this functions sets a global error-number (ipFuncErrno) if an error
 *   occured     
 *
 *   @param error_no  number which describes the error
 *
 *   @return none
 *
 * AUTHOR & DATE
 */

/* include files */

#include "ipFuncP.h"


#ifndef DOXYGEN_IGNORE

#ifndef lint
  static char *what = { "@(#)_ipFuncSetErrno\t\tDKFZ (Dept. MBI)\t"__DATE__ };
#endif


/* gloabal variables */

int ipFuncErrno;

/* ------------------------------------------------------------------- */

/*
** _ipFuncSetErrno
*/

void _ipFuncSetErrno ( int error_no )
{
   ipFuncErrno = error_no;

   return;
}

/* ------------------------------------------------------------------- */

/*
** ipFuncPError       
*/


void ipFuncPError ( char *string )
{
   char *err_string=NULL;

   switch ( ipFuncErrno )
     {
        case mitkIpFuncOK :
             err_string = "ipFunc: no error occured";
             break;
        case mitkIpFuncNOPIC_ERROR :
             err_string = "no image";              
             break;
        case mitkIpFuncDIM_ERROR :
             err_string = "ipFunc: incorrect dimension of image";
             break;
        case mitkIpFuncSIZE_ERROR :
             err_string = "ipFunc: incorrect image size";
             break;
        case mitkIpFuncFLAG_ERROR :
             err_string = "ipFunc: incorrect flag";
             break;
        case mitkIpFuncMALLOC_ERROR :
             err_string = "ipFunc: no memory allocated";
             break;
        case mitkIpFuncPICNEW_ERROR :
             err_string = "ipFunc: can't create pic_new";
             break;
        case mitkIpFuncTYPE_ERROR :
             err_string = "ipFunc: incorrect image data type ";
             break;
        case mitkIpFuncUNFIT_ERROR :
             err_string = "ipFunc: image data sizeis don't correspond";
             break;
        case mitkIpFuncDIMMASC_ERROR :
             err_string = "ipFunc: incorrect mask dimension";
             break;
        case mitkIpFuncDATA_ERROR :
             err_string = "ipFunc: incorrect value of parameter";
             break;
        default :
             err_string = "ipFunc: unknown error";
     }
   fprintf ( stderr, "error message: %s \n", err_string );
}

#endif

