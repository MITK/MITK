
/*
 * ipPicAddT.c
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   inserts a tsv into the pic
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1994 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicAddTag\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"


void
ipPicAddTag( ipPicDescriptor *pic, ipPicTSV_t *tsv )
{
  pic->info->tags_head = _ipPicInsertTag( pic->info->tags_head, tsv );
}

void
ipPicAddSubTag( ipPicTSV_t *parent, ipPicTSV_t *tsv )
{
  if( parent->type != ipPicTSV )
    return;

  parent->n[0]++;
  parent->value = _ipPicInsertTag( parent->value, tsv );
}

_ipPicTagsElement_t *
_ipPicInsertTag( _ipPicTagsElement_t *head, ipPicTSV_t *tsv )
{
  int i;
  _ipPicTagsElement_t *new;
  _ipPicTagsElement_t *current;

  tsv->tag[_ipPicTAGLEN] = '\0';
  for( i=strlen(tsv->tag); i<_ipPicTAGLEN; i++ )
    tsv->tag[i] = ' ';

  new = malloc( sizeof(_ipPicTagsElement_t) );

  new->tsv = tsv;
  new->prev = NULL;
  new->next = NULL;

  if( head == NULL )
    {
      head = new;
    }
  else
    {
      current = head;

      while( current->next != NULL )
        {
          current = current->next;
        }

      new->prev = current;
      current->next = new;
    }

  return( head );
}
