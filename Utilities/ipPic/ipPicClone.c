/** @file
 *  creates the clone of a pic
 */
#ifndef lint
  static char *what = { "@(#)ipPicClone\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

/**
 * creates the clone of a pic
 * @param pic		the original pic
 *
 * @return the exact copy
 *
 *
 * AUTHOR & DATE
 *
 */

ipPicDescriptor *ipPicClone( ipPicDescriptor *pic )
{
  ipPicDescriptor *clone;

  clone = ipPicCopyHeader( pic,
                           NULL );

  strncpy( clone->info->version, pic->info->version, _ipPicTAGLEN );

  clone->info->tags_head = _ipPicCloneTags( pic->info->tags_head );

  clone->info->write_protect = pic->info->write_protect;

  if( !pic->data )
    {
      clone->data = NULL;
    }
  else
    {
      ipUInt4_t size;

      size = _ipPicSize( pic );

      clone->data = malloc( size );
      memcpy( clone->data, pic->data, size );
    }

  return( clone );
}

_ipPicTagsElement_t *_ipPicCloneTags( _ipPicTagsElement_t *head )
{
  _ipPicTagsElement_t *new;
  _ipPicTagsElement_t *current;

  new = NULL;
  current = head;
  while( current != NULL )
    {
      new = _ipPicInsertTag( new, _ipPicCloneTag(current->tsv) );

      current = current->next;
    }
  return( new );
}

ipPicTSV_t *_ipPicCloneTag( ipPicTSV_t *source )
{
  ipPicTSV_t *new;

  ipUInt4_t i;

  new = malloc( sizeof(ipPicTSV_t) );

  strncpy( new->tag, source->tag, _ipPicTAGLEN );

  new->type = source->type;
  new->bpe = source->bpe;
  new->dim = source->dim;

  for( i = 0; i < source->dim; i++ )
    new->n[i] = source->n[i];

  if( source->type == ipPicTSV )
    {
      new->value = _ipPicCloneTags( source->value );
    }
  else
    {
      ipUInt4_t  size;

      size = _ipPicTSVSize( new );

      new->value = malloc( size );
      memcpy( new->value, source->value, size );
    }

  return( new );
}
