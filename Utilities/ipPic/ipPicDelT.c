
/*
 * ipPicDelT.c
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   inserts a tsv into the pic
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1994 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicDelTag\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

static ipPicTSV_t *_ipPicRemoveTag( _ipPicTagsElement_t **head,
                                    _ipPicTagsElement_t *which,
                                    char *tag );

ipPicTSV_t *
ipPicDelTag( ipPicDescriptor *pic, char *tag )
{
  int i;
  _ipPicTagsElement_t *which;

  which = _ipPicFindTag( pic->info->tags_head, tag );

  return( _ipPicRemoveTag( &(pic->info->tags_head), which, tag ) );
}

ipPicTSV_t *
ipPicDelSubTag( ipPicTSV_t *parent, char *tag )
{
  int i;
  _ipPicTagsElement_t *which;

  if( parent->type != ipPicTSV )
    return( NULL );

  which = _ipPicFindTag( parent->value, tag );

  if( !which )
    return( NULL );

  if( which->tsv->type == ipPicTSV )
    return( NULL );

  parent->n[0]--;
  return( _ipPicRemoveTag( (_ipPicTagsElement_t **)&(parent->value), which, tag ) );
}

ipPicTSV_t *
_ipPicRemoveTag( _ipPicTagsElement_t **head, _ipPicTagsElement_t *which, char *tag )
{
  ipPicTSV_t *tsv = NULL;

  if( which != NULL )
    {
      tsv = which->tsv;

      if( which->prev == NULL )              /* which is the current head */
        {
          *head = which->next;

          if( *head )
            (*head)->prev = NULL;
        }
      else if( which->next == NULL )         /* which is the current tail */
        {
          which->prev->next = NULL;
        }
      else                                   /* which is somewhere if the list */
        {
          which->prev->next = which->next;
          which->next->prev = which->prev;
        }

      free( which );
    }

  return( tsv );
}
