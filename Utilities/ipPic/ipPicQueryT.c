/*
 * ipPicQueryT.c
 *---------------------------------------------------------------------
 * DESCRIPTION
 *  searches for a tag
 *
 * FUNCTION DECLARATION
 *  ipPicTSV_t *ipPicQueryTag( ipPicDescriptor *pic, char *tag )
 *  _ipPicTagsElement_t *_ipPicFindTag( _ipPicTagsElement_t *head, char *tag )
 *
 * PARAMETERS
 *  pic			the pic
 *  tag			the tag to search for
 *  head		the head of the tags list
 *
 * RETURN VALUES
 *  
 *  
 *
 * AUTHOR & DATE
 *  Andre Schroeter	27.08.94
 *
 * UPDATES
 *  none
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1994 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicQueryTag\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

ipPicTSV_t *
ipPicQueryTag( ipPicDescriptor *pic, char *tag )
{
  _ipPicTagsElement_t *found;

  found = _ipPicFindTag( pic->info->tags_head, tag );

  if( found == NULL )
    return( NULL );
  else
    return( found->tsv );
}
ipPicTSV_t *
ipPicQuerySubTag( ipPicTSV_t *parent, char *tag )
{
  _ipPicTagsElement_t *found;

  found = _ipPicFindTag( parent->value, tag );

  if( found == NULL )
    return( NULL );
  else
    return( found->tsv );
}

_ipPicTagsElement_t *
_ipPicFindTag( _ipPicTagsElement_t *head, char *tag )
{
  int i;
  char name[_ipPicTAGLEN+1];
  _ipPicTagsElement_t *current;

  if( !tag )
    return( NULL );

  sprintf( name, "%.*s", _ipPicTAGLEN, tag );
  for( i=strlen(name); i<_ipPicTAGLEN; i++ )
    name[i] = ' ';

  current = head;
  while( current != NULL )
    {
      if( strncmp( current->tsv->tag, name, _ipPicTAGLEN) == 0 )
        return( current );

      current = current->next;
    }

  return( NULL );
}
