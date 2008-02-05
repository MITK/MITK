/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/
#include <assert.h>
#include "empty.xpm"
#include "ipSegmentationP.h"

void 
ipMITKSegmentationClear (ipPicDescriptor* segmentation)
{
  ipMITKSegmentationTYPE *cur, *last; 
  ipPicTSV_t* tag; 
  const char *src; 
  ipUInt1_t *dst;
  ipUInt4_t i, j;

  assert (segmentation);
  if (ipMITKSegmentationUndoIsEnabled (segmentation)) {
    ipMITKSegmentationUndoSave (segmentation);
  }

  /* clear the image data */

  cur = (ipMITKSegmentationTYPE *) segmentation->data;
  last = cur + _ipPicElements (segmentation);
  while (cur < last) {
    *cur++ = 0;
  }

  /* create an 'empty' tag */

  tag = ipPicQueryTag (segmentation, tagSEGMENTATION_EMPTY);
  if (!tag) {
    tag = (ipPicTSV_t *) malloc (sizeof (ipPicTSV_t));
    strcpy (tag->tag, tagSEGMENTATION_EMPTY);
    tag->type = ipPicBool;
    tag->bpe = sizeof (ipBool_t) / 8;
    tag->dim = 1;
    tag->n[0] = 1;
    tag->value = malloc (sizeof (ipBool_t)); 
    *((ipBool_t *) tag->value) = ipTrue;
    ipPicAddTag (segmentation, tag);
  }

  /* create an `empty` icon */

  tag = ipPicQueryTag (segmentation, "ICON80x80");
  if (!tag) {
    tag = (ipPicTSV_t *) malloc (sizeof (ipPicTSV_t));
    tag->type = ipPicUInt;
    tag->bpe = 8;
    tag->dim = 2;
    tag->n[0] = 80;
    tag->n[1] = 80;  
    tag->value = malloc (tag->n[0] * tag->n[1] * sizeof (ipUInt1_t));
    strcpy (tag->tag, "ICON80x80");
    ipPicAddTag (segmentation, tag);
  }
  dst = (ipUInt1_t *) tag->value;
  for (i = 0; i < 80; i++) {
    src = empty_xpm [3+i];
    for (j = 0; j < 80; j++) {
      *dst = (*src == '.' ? 0 : 196);
      src++;
      dst++;
    }
  }    
}
