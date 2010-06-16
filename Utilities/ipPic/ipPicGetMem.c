
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

mitkIpPicDescriptor *mitkIpPicGetMem( mitkIpUInt1_t *mem_pic )
{
  mitkIpPicDescriptor *pic;
  mitkIpUInt4_t        len;
  mitkIpUInt4_t        to_read;
  mitkIpUInt1_t       *mem_ptr = mem_pic;

  if( !mem_pic )
    return( NULL );
  
  pic = mitkIpPicNew();
  mitkIpPicClear( pic );
  memmove( &(pic->info->version), mem_ptr, sizeof(mitkIpPicTag_t) );
  mem_ptr += sizeof(mitkIpPicTag_t); 
  if( strncmp( mitkIpPicVERSION, pic->info->version, 4 ) != 0 )
    {      
      mitkIpPicFree( pic );
      return( NULL );
    }

  /*memmove( &len,  mem_ptr, sizeof(mitkIpUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &len, sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
  mem_ptr += sizeof(mitkIpUInt4_t);

  /*memmove( &(pic->type),  mem_ptr, sizeof(mitkIpUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &(pic->type), sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
  mem_ptr += sizeof(mitkIpUInt4_t);

  /*memmove( &(pic->bpe),  mem_ptr, sizeof(mitkIpUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &(pic->bpe), sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
  mem_ptr += sizeof(mitkIpUInt4_t);

  /*memmove( &(pic->dim),  mem_ptr, sizeof(mitkIpUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &(pic->dim), sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
  mem_ptr += sizeof(mitkIpUInt4_t);

  /*memmove( &(pic->n),  mem_ptr, pic->dim * sizeof(mitkIpUInt4_t) );*/
  mitkIpCpFromLE( mem_ptr, &(pic->n), pic->dim*sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
  mem_ptr += pic->dim*sizeof(mitkIpUInt4_t);

  to_read = len -        3 * sizeof(mitkIpUInt4_t)
                - pic->dim * sizeof(mitkIpUInt4_t);
  pic->info->tags_head = _mitkIpPicReadTagsMem( pic->info->tags_head, 
					    to_read, &mem_ptr,
                                            mitkIpPicEncryptionType(pic) );

  pic->info->write_protect = mitkIpFalse;
  pic->data = malloc( _mitkIpPicSize(pic) );

  /*
    pic->info->pixel_start_in_file = ftell( infile );
  */
  
  /*memmove( pic->data,  mem_ptr, pic->bpe/8*_mitkIpPicElements(pic) );*/
  mitkIpCpFromLE( mem_ptr, pic->data, _mitkIpPicSize(pic), pic->bpe/8 );

  return( pic );
}

_mitkIpPicTagsElement_t *
_mitkIpPicReadTagsMem( _mitkIpPicTagsElement_t *head, mitkIpUInt4_t bytes_to_read, 
		   mitkIpUInt1_t **mem_ptr, char encryption_type )
{
  while( bytes_to_read > 0 )
    {
      mitkIpPicTSV_t *tsv;
      mitkIpUInt4_t   len;

      tsv = malloc( sizeof(mitkIpPicTSV_t) );

      memmove( tsv->tag, *mem_ptr, _mitkIpPicTAGLEN );
      tsv->tag[_mitkIpPicTAGLEN] = '\0';
      *mem_ptr += _mitkIpPicTAGLEN;

      /*memmove( &len,  *mem_ptr, sizeof(mitkIpUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &len, sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
      *mem_ptr += sizeof(mitkIpUInt4_t);

      /*memmove( &(tsv->type), *mem_ptr, sizeof(mitkIpUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &(tsv->type), sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
      *mem_ptr += sizeof(mitkIpUInt4_t);

      /*memmove( &(tsv->bpe), *mem_ptr, sizeof(mitkIpUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &(tsv->bpe), sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
      *mem_ptr += sizeof(mitkIpUInt4_t);

      /*memmove( &(tsv->dim), *mem_ptr, sizeof(mitkIpUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &(tsv->dim), sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
      *mem_ptr += sizeof(mitkIpUInt4_t);

      /*memmove( &(tsv->n), *mem_ptr, tsv->dim*sizeof(mitkIpUInt4_t) );*/
      mitkIpCpFromLE( *mem_ptr, &(tsv->n), tsv->dim*sizeof(mitkIpUInt4_t), sizeof(mitkIpUInt4_t) );
      *mem_ptr += tsv->dim*sizeof(mitkIpUInt4_t);

      if( tsv->type == mitkIpPicTSV )
        {
          tsv->value = NULL;
          tsv->value = _mitkIpPicReadTagsMem( tsv->value,
					  len -        3 * sizeof(mitkIpUInt4_t)
  					      - tsv->dim * sizeof(mitkIpUInt4_t),
					  mem_ptr,
                                          encryption_type );
        }
      else
        {
          mitkIpUInt4_t  elements;

          elements = _mitkIpPicTSVElements( tsv );

          if( tsv->type == mitkIpPicASCII
              || tsv->type == mitkIpPicNonUniform )
            tsv->bpe = 8;

          if( tsv->type == mitkIpPicASCII )
            tsv->value = malloc( (elements+1) * (tsv->bpe / 8) );
          else
            tsv->value = malloc( elements * tsv->bpe / 8 );

	  /*memmove( tsv->value,  *mem_ptr, tsv->bpe/8*_mitkIpPicTSVElements(tsv) );*/
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
             tsv->n[0] = (mitkIpUInt4_t)strlen(tsv->value);
             tsv->type = mitkIpPicASCII;
             tsv->dim = 1;
           }
        }

      head = _mitkIpPicInsertTag( head, tsv );

      bytes_to_read -= 32                  /* name      */
                       + sizeof(mitkIpUInt4_t) /* len       */
                       + len;              /* type + bpe + dim + n[] + data */
    }
  return( head );
}
