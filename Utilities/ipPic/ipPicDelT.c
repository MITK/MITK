
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

static _ipPicTagsElement_t *_ipPicRemoveTag( _ipPicTagsElement_t *head, char *tag );

void
ipPicDelTag( ipPicDescriptor *pic, char *t )
{
  int i;
  char tag[_ipPicTAGLEN];

  sprintf( tag, "%.*s", _ipPicTAGLEN, t );
  for( i=strlen(tag); i<_ipPicTAGLEN; i++ )
    tag[i] = ' ';

  pic->info->tags_head = _ipPicRemoveTag( pic->info->tags_head, tag );
}

_ipPicTagsElement_t *
_ipPicRemoveTag( _ipPicTagsElement_t *head, char *tag )
{
  _ipPicTagsElement_t *which;

  which = _ipPicFindTag( head, tag );

  if( which != NULL )
    {
      if( which->prev == NULL )              /* which is the current head */
        {
          head = which->next;
          head->prev = NULL;
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
  return( head );
}
