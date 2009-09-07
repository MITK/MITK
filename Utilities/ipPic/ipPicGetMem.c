
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
  static char *what = { "@(#)mitkIpPicGetMem\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "mitkIpPic.h"

mitkIpPicDescriptor *mitkIpPicGetMem( ipUInt1_t *mem_pic )
{
  mitkIpPicDescriptor *pic;
  ipUInt4_t        len;
  ipUInt4_t        to_read;
  ipUInt1_t       *mem_ptr = mem_pic;

  if( !mem_pic )
    return( NULL );
  
  pic = mitkIpPicNew();
  mitkIpPicClear( pic );
  memcpy( &(pic->info->version), mem_ptr, sizeof(mitkIpPicTag_t) );
  mem_ptr += sizeof(mitkIpPicTag_t); 
  if( strncmp( mitkIpPicVERSION, pic->info->version, 4 ) != 0 )
    {      
      mitkIpPicFree( pic );
      return( NULL );
    }

  /*memcpy( &len,  mem_ptr, sizeof(ipUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &len, sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);

  /*memcpy( &(pic->type),  mem_ptr, sizeof(ipUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &(pic->type), sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);

  /*memcpy( &(pic->bpe),  mem_ptr, sizeof(ipUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &(pic->bpe), sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);

  /*memcpy( &(pic->dim),  mem_ptr, sizeof(ipUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &(pic->dim), sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
  mem_ptr += sizeof(ipUInt4_t);

  /*memcpy( &(pic->n),  mem_ptr, pic->dim * sizeof(ipUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &(pic->n), pic->dim*sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
  mem_ptr += pic->dim*sizeof(ipUInt4_t);

  to_read = len -        3 * sizeof(ipUInt4_t)
                - pic->dim * sizeof(ipUInt4_t);
  pic->info->tags_head = _mitkIpPicReadTagsMem( pic->info->tags_head, 
					    to_read, &mem_ptr,
                                            mitkIpPicEncryptionType(pic) );

  pic->info->write_protect = ipFalse;
  pic->data = malloc( _mitkIpPicSize(pic) );

  /*
    pic->info->pixel_start_in_file = ftell( infile );
  */
  
  /*memcpy( pic->data,  mem_ptr, pic->bpe/8*_mitkIpPicElements(pic) );*/
  mitkIpCpFromLE( mem_ptr, pic->data, _mitkIpPicSize(pic), pic->bpe/8 );

  return( pic );
}

_mitkIpPicTagsElement_t *
_mitkIpPicReadTagsMem( _mitkIpPicTagsElement_t *head, ipUInt4_t bytes_to_read, 
		   ipUInt1_t **mem_ptr, char encryption_type )
{
  while( bytes_to_read > 0 )
    {
      mitkIpPicTSV_t *tsv;
      ipUInt4_t   len;

      tsv = malloc( sizeof(mitkIpPicTSV_t) );

      memcpy( tsv->tag, *mem_ptr, _mitkIpPicTAGLEN );
      tsv->tag[_mitkIpPicTAGLEN] = '\0';
      *mem_ptr += _mitkIpPicTAGLEN;

      /*memcpy( &len,  *mem_ptr, sizeof(ipUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &len, sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);

      /*memcpy( &(tsv->type), *mem_ptr, sizeof(ipUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &(tsv->type), sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);

      /*memcpy( &(tsv->bpe), *mem_ptr, sizeof(ipUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &(tsv->bpe), sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);

      /*memcpy( &(tsv->dim), *mem_ptr, sizeof(ipUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &(tsv->dim), sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
      *mem_ptr += sizeof(ipUInt4_t);

      /*memcpy( &(tsv->n), *mem_ptr, tsv->dim*sizeof(ipUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &(tsv->n), tsv->dim*sizeof(ipUInt4_t), sizeof(ipUInt4_t) );
      *mem_ptr += tsv->dim*sizeof(ipUInt4_t);

      if( tsv->type == mitkIpPicTSV )
        {
          tsv->value = NULL;
          tsv->value = _mitkIpPicReadTagsMem( tsv->value,
					  len -        3 * sizeof(ipUInt4_t)
  					      - tsv->dim * sizeof(ipUInt4_t),
					  mem_ptr,
                                          encryption_type );
        }
      else
        {
          ipUInt4_t  elements;

          elements = _mitkIpPicTSVElements( tsv );

          if( tsv->type == mitkIpPicASCII
              || tsv->type == mitkIpPicNonUniform )
            tsv->bpe = 8;

          if( tsv->type == mitkIpPicASCII )
            tsv->value = malloc( (elements+1) * (tsv->bpe / 8) );
          else
            tsv->value = malloc( elements * tsv->bpe / 8 );

	  /*memcpy( tsv->value,  *mem_ptr, tsv->bpe/8*_mitkIpPicTSVElements(tsv) );*/
          mitkIpCpFromLE( *mem_ptr, tsv->value, _mitkIpPicTSVSize(tsv), tsv->bpe/8 );
	  *mem_ptr += tsv->bpe/8*_mitkIpPicTSVElements(tsv);
	  
          if( tsv->type == mitkIpPicASCII )
            ((char *)(tsv->value))[elements] = '\0';

         if( encryption_type == 'e'
             && ( tsv->type == mitkIpPicASCII
                  || tsv->type == mitkIpPicNonUniform ) )
           {
             if( tsv->type == mitkIpPicNonUniform )
               {
                  sprintf( tsv->tag, "*** ENCRYPTED ***" );
                  tsv->tag[_mitkIpPicTAGLEN] = '\0';
               }
  
             if( tsv->value )
               free( tsv->value );
  
             tsv->value = strdup( "*** ENCRYPTED ***" );
             tsv->n[0] = strlen(tsv->value);
             tsv->type = mitkIpPicASCII;
             tsv->dim = 1;
           }
        }

      head = _mitkIpPicInsertTag( head, tsv );

      bytes_to_read -= 32                  /* name      */
                       + sizeof(ipUInt4_t) /* len       */
                       + len;              /* type + bpe + dim + n[] + data */
    }
  return( head );
}
