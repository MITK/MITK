
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
