
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   Clears the entries in a picDescriptor
 *   and frees the data and info
 *
 * $Log$
 * Revision 1.3  2000/05/04 12:35:58  ivo
 * some doxygen comments.
 *
 * Revision 1.2  1997/09/15  13:21:10  andre
 * switched to new what string format
 *
 * Revision 1.1.1.1  1997/09/06  19:09:58  andre
 * initial import
 *
 * Revision 0.1  1993/04/02  16:18:39  andre
 * now works on PC, SUN and DECstation
 *
 * Revision 0.0  1993/03/26  12:56:26  andre
 * Initial revision, NO error checking
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicClear\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

void ipPicClear( ipPicDescriptor *pic )
{
  if( pic != NULL )
    {
      if( pic->data != NULL )
        {
          free( pic->data );
          pic->data = NULL;
        }

      if( pic->info != NULL )
        {
          _ipPicFreeTags( pic->info->tags_head );
          pic->info->tags_head = NULL;

          /*free( pic->info );
          pic->info = NULL;*/
        }
      pic->type = ipPicUnknown;
    }
}

void _ipPicFreeTags( _ipPicTagsElement_t *head )
{
  _ipPicTagsElement_t *current;

  current = head;
  while( current != NULL )
    {
      _ipPicTagsElement_t *this = current;
      current = current->next;

      if( this->tsv != NULL )
        {
          if( this->tsv->value != NULL )
            {
              if( this->tsv->type == ipPicTSV )
                _ipPicFreeTags( this->tsv->value );
              else
                free( this->tsv->value );
            }
          free( this->tsv );
        }

      free( this );
    }
}
