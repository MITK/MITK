
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

  printf( "%i\n", _ipEndian() );

  if( argc != 3 )
    exit(-1);

  pic = ipPicGet( argv[1],
                  NULL );

  /*tsv = malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv->tag, "MIN/MAX");
  tsv->type = ipPicUInt;
  tsv->bpe = 8;
  tsv->dim = 1;
  tsv->n[0] = 2;
  tsv->value = malloc( 1*2 );
  ((ipUInt1_t *)tsv->value)[0] = 0;
  ((ipUInt1_t *)tsv->value)[1] = 100;
  ipPicAddTag( pic, tsv ); /**/

  tsv = malloc( sizeof(ipPicTSV_t) );
  strcpy( tsv->tag, "LEVEL/WINDOW");
  tsv->type = ipPicInt;
  tsv->bpe = 16;
  tsv->dim = 1;
  tsv->n[0] = 2;
  tsv->value = malloc( (tsv->bpe/8)*2 );
  ((ipInt2_t *)tsv->value)[0] = 0;
  ((ipInt2_t *)tsv->value)[1] = 300;
  ipPicAddTag( pic, tsv ); /**/

  ipPicPut( argv[2],
            pic );

  return(0);
}
