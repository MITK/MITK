
/*
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   creates a PicStructure from memory block
 *
 * FUNCTION DECLARATION
 *  
 *
 * PARAMETERS
 *  
 *
 * AUTHOR & DATE
 *  Harald Evers   15.7.98
 *
 * UPDATES
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1998 by DKFZ (Dept. MBI) Heidelberg, FRG
 */

#ifndef lint
  static char *what = { "@(#)ipPicGetMem\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

ipPicDescriptor *ipPicGetMem( ipUInt1_t *mem_pic )
{
  ipPicDescriptor *pic;
  ipUInt4_t        len;
  ipUInt4_t        to_read;
  ipUInt1_t       *mem_ptr = mem_pic;
  
  pic = ipPicNew();
  ipPicClear( pic );
  memcpy( &(pic->info->version), mem_ptr, sizeof(ipPicTag_t) );
  mem_ptr += sizeof(ipPicTag_t); 
  if( strncmp( ipPicVERSION, pic->info->version, 4 ) != 0 )
    {      
      ipPicFree( pic );
      return( NULL );
    }

  memcpy( &len,  mem_ptr, sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( &(pic->type),  mem_ptr, sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( &(pic->bpe),  mem_ptr, sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( &(pic->dim),  mem_ptr, sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);
  memcpy( &(pic->n),  mem_ptr, sizeof(ipUInt4_t) );
  mem_ptr += pic->dim*sizeof(ipUInt4_t);

  to_read = len -        3 * sizeof(ipUInt4_t)
                - pic->dim * sizeof(ipUInt4_t);
  pic->info->tags_head = _ipPicReadTagsMem( pic->info->tags_head, 
					    to_read, &mem_ptr );

  pic->info->write_protect = ipFalse;
  pic->data = malloc( _ipPicSize(pic) );

  /*
    pic->info->pixel_start_in_file = ftell( infile );
  */
  
  memcpy( pic->data,  mem_ptr, pic->bpe/8*_ipPicElements(pic) );

  return( pic );
}

_ipPicTagsElement_t *
_ipPicReadTagsMem( _ipPicTagsElement_t *head, ipUInt4_t bytes_to_read, 
		   ipUInt1_t **mem_ptr )
{
  while( bytes_to_read > 0 )
    {
      ipPicTSV_t *tsv;
      ipUInt4_t   len;

      tsv = malloc( sizeof(ipPicTSV_t) );

      memcpy( tsv->tag, *mem_ptr, _ipPicTAGLEN );
      tsv->tag[_ipPicTAGLEN] = '\0';
      *mem_ptr += _ipPicTAGLEN;

      memcpy( &len,  *mem_ptr, sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);

      memcpy( &(tsv->type), *mem_ptr, sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( &(tsv->bpe), *mem_ptr, sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( &(tsv->dim), *mem_ptr, sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);
      memcpy( &(tsv->n), *mem_ptr, tsv->dim*sizeof(ipUInt4_t) );
      *mem_ptr += tsv->dim*sizeof(ipUInt4_t);

      if( tsv->type == ipPicTSV )
        {
          tsv->value = NULL;
          tsv->value = _ipPicReadTagsMem( tsv->value,
					  len -        3 * sizeof(ipUInt4_t)
  					      - tsv->dim * sizeof(ipUInt4_t),
					  mem_ptr );
        }
      else
        {
          ipUInt4_t  elements;

          elements = _ipPicTSVElements( tsv );

          if( tsv->type == ipPicASCII
              || tsv->type == ipPicNonUniform )
            tsv->bpe = 8;

          if( tsv->type == ipPicASCII )
            tsv->value = malloc( elements+1 * tsv->bpe / 8 );
          else
            tsv->value = malloc( elements * tsv->bpe / 8 );

	  memcpy( tsv->value,  *mem_ptr, tsv->bpe/8*_ipPicTSVElements(tsv) );
	  *mem_ptr += tsv->bpe/8*_ipPicTSVElements(tsv);
	  
          if( tsv->type == ipPicASCII )
            ((char *)(tsv->value))[elements] = '\0';

          /*
	    if( encryption_type == 'e' ..
	  */
        }

      head = _ipPicInsertTag( head, tsv );

      bytes_to_read -= 32                  /* name      */
                       + sizeof(ipUInt4_t) /* len       */
                       + len;              /* type + bpe + dim + n[] + data */
    }
  return( head );
}
