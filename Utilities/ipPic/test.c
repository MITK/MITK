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


#include <stdio.h>
#include <strings.h>

#include "ipPic.h"
#include "ipPicAnnotation.h"

int main( int argc, char *argv[] )
{
  ipPicDescriptor *pic;
  ipPicDescriptor *pic2;
  ipPicTSV_t *tsv;
  ipPicTSV_t *tsv2;
  ipPicTSV_t *tsv3;
  ipPicAnnotation_t	a;
  char buff[] = "test test";

  printf( "%s\n", _ipEndian()==_ipEndianBig?"BigEndian":"LittleEndian" );

  if( argc != 3 )
    exit(-1);

  pic = ipPicGet( argv[1],
                  NULL );

  pic->info->tags_head = NULL;
  pic->dim = 2;
  pic->n[0] = 90;
  pic->n[1] = 90;
  ipPicPutSlice( argv[2],
            pic, 1 );
  ipPicPutSlice( argv[2],
            pic, 2 );
exit(1);



  a.type = ipPicAText;
  a.location_x = 30;
  a.location_y = 35;
  a.position_x = 64;
  a.position_y = 46;

  tsv = malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv->tag, "ANNOTATION");
  tsv->type = ipPicTSV;
  tsv->bpe = 32;
  tsv->dim = 0;
  tsv->n[0] = 0;
  tsv->value = NULL;
  ipPicAddTag( pic, tsv ); /**/

  tsv2 = malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv2->tag, "TYPE");
  tsv2->type = ipPicUInt;
  tsv2->bpe = 32;
  tsv2->dim = 1;
  tsv2->n[0] = 1;
  tsv2->value = malloc( 4 );
  *(ipUInt4_t *)tsv2->value = a.type;
  ipPicAddSubTag( tsv, tsv2 );

  tsv2 = malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv2->tag, "LOCATION");
  tsv2->type = ipPicUInt;
  tsv2->bpe = 32;
  tsv2->dim = 1;
  tsv2->n[0] = 2;
  tsv2->value = malloc( 4*2 );
  ((ipUInt4_t *)tsv2->value)[0] = a.location_x;
  ((ipUInt4_t *)tsv2->value)[1] = a.location_x;
  ipPicAddSubTag( tsv, tsv2 );

  tsv2 = malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv2->tag, "POSITION ");
  tsv2->type = ipPicUInt;
  tsv2->bpe = 32;
  tsv2->dim = 1;
  tsv2->n[0] = 2;
  tsv2->value = malloc( 4*2 );
  ((ipUInt4_t *)tsv2->value)[0] = a.position_x;
  ((ipUInt4_t *)tsv2->value)[1] = a.position_y;
  ipPicAddSubTag( tsv, tsv2 );

  tsv2 = malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv2->tag, "TEXT");
  tsv2->type = ipPicASCII;
  tsv2->bpe = 8;
  tsv2->dim = 1;
  tsv2->n[0] = strlen(buff);
  tsv2->value = malloc( strlen(buff) );
  strncpy( tsv2->value, buff, strlen(buff) );
  ipPicAddSubTag( tsv, tsv2 );

  tsv3 = malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv3->tag, "ANNOTATION");
  tsv3->type = ipPicTSV;
  tsv3->bpe = 32;
  tsv3->dim = 1;
  tsv3->n[0] = 0;
  tsv3->value = NULL;
  ipPicAddSubTag( tsv, tsv3 );

  tsv2 = malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv2->tag, "X1X2X3");
  tsv2->type = ipPicUInt;
  tsv2->bpe = 32;
  tsv2->dim = 1;
  tsv2->n[0] = 1;
  tsv2->value = malloc( 4 );
  *(ipUInt4_t *)tsv2->value = a.type;

  ipPicAddSubTag( tsv3, tsv2 );

  printf( "%i\n", ipPicQuerySubTag( tsv3, "X1X2X32" ) );

  ipPicDelSubTag( tsv, "TYPE" );

  ipPicPut( argv[2],
            pic );

  pic2 = ipPicClone( pic );

  ipPicPut( "xxx.pic",
            pic2 ); /**/


  ipPicFree( pic );
  ipPicFree( pic2 );

  return(0);
}
