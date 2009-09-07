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

#include <string.h>
#include <assert.h>
#include "ipSegmentationP.h"

static char* tagUNDO = "SEGMENTATION_UNDO";
static char* tagUNDO_DATA = "DATA";
static char* tagUNDO_LEVEL = "LEVEL";

// Implemented in mitkIpPicDelT.c 
static mitkIpPicTSV_t *_mitkIpPicRemoveTag( _mitkIpPicTagsElement_t **head,
                                    _mitkIpPicTagsElement_t *which,
                                    char *tag );

void
ipMITKSegmentationUndoEnable (mitkIpPicDescriptor* segmentation, const ipUInt1_t level)
{
    mitkIpPicTSV_t *undo, *data, *max;

    assert (segmentation);
    undo = mitkIpPicQueryTag (segmentation, tagUNDO);
    if (!undo) {
        undo = (mitkIpPicTSV_t *) malloc (sizeof (mitkIpPicTSV_t));
        if (!undo) {
            ipMITKSegmentationError (ipMITKSegmentationOUT_OF_MEMORY);
        }
        strcpy (undo->tag, tagUNDO);
        undo->type = mitkIpPicTSV;
        undo->bpe = 32;
        undo->dim = 1;
        undo->n[0] = 0;
        undo->value = NULL;
        mitkIpPicAddTag (segmentation, undo);
    }

    data = mitkIpPicQuerySubTag (undo, tagUNDO_DATA);
    if (!data) {
        data = (mitkIpPicTSV_t *) malloc (sizeof (mitkIpPicTSV_t));
        if (!data) {
            ipMITKSegmentationError (ipMITKSegmentationOUT_OF_MEMORY);
        }
        strcpy (data->tag, tagUNDO_DATA);
        data->type = mitkIpPicTSV;
        data->bpe = 32;
        data->dim = 1;
        data->n[0] = 0;
        data->value = NULL;
        mitkIpPicAddSubTag (undo, data);
    }
    if (data->n[0] > level) {
        /* remove levels which exceed the maximum */
        ipUInt1_t i;

        for (i = data->n[0] - level; i > 0; i--) {
            _mitkIpPicTagsElement_t *head = (_mitkIpPicTagsElement_t *) data->value;
            mitkIpPicTSV_t* tag = _mitkIpPicRemoveTag (&head, head, head->tsv->tag);
      data->value = head;
      data->n[0]--;
            mitkIpPicFreeTag (tag);
        }
    }

    max = mitkIpPicQuerySubTag (undo, tagUNDO_LEVEL);
    if (max) {
        /* change the maximum of levels */
        ipUInt1_t* value = (ipUInt1_t *) max->value;
        *value = level;
    } else {
        ipUInt1_t* value = (ipUInt1_t *) malloc (sizeof (ipUInt1_t));
        if (!value) {
            ipMITKSegmentationError (ipMITKSegmentationOUT_OF_MEMORY);
        }
        *value = level;

        max = (mitkIpPicTSV_t *) malloc (sizeof (mitkIpPicTSV_t));
        if (!max) {
            ipMITKSegmentationError (ipMITKSegmentationOUT_OF_MEMORY);
        }
        strcpy (max->tag, tagUNDO_LEVEL);
        max->type = mitkIpPicUInt;
        max->bpe = 8;
        max->dim = 1;
        max->n[0] = 1;
        max->value = value;
        mitkIpPicAddSubTag (undo, max);
    }
}

void
ipMITKSegmentationUndoDisable (mitkIpPicDescriptor* segmentation)
{
    mitkIpPicTSV_t *undo;

    assert (segmentation);
    undo = mitkIpPicDelTag (segmentation, tagUNDO);
    mitkIpPicFreeTag (undo);
}

ipBool_t
ipMITKSegmentationUndoIsEnabled (mitkIpPicDescriptor* segmentation)
{
    mitkIpPicTSV_t *undo = NULL;

    if (segmentation) {
  undo = mitkIpPicQueryTag (segmentation, tagUNDO);
    }
    return (undo ? ipTrue : ipFalse);
}

void
ipMITKSegmentationUndoSave (mitkIpPicDescriptor* segmentation)
{
    mitkIpPicTSV_t *undo, *data, *level, *tag;

    assert (segmentation);
    undo = mitkIpPicQueryTag (segmentation, tagUNDO);
    if (!undo) {
        ipMITKSegmentationError (ipMITKSegmentationUNDO_DISABLED);
    }

    /* if no level is available ... */
    data = mitkIpPicQuerySubTag (undo, tagUNDO_DATA);
    level = mitkIpPicQuerySubTag (undo, tagUNDO_LEVEL);
    if (*((ipUInt1_t *) level->value) > 0) {
        if (data->n[0] == *((ipUInt1_t *) level->value)) {
            /* ... remove the first one. */
            _mitkIpPicTagsElement_t* head = (_mitkIpPicTagsElement_t *) data->value;
            mitkIpPicTSV_t* tag = _mitkIpPicRemoveTag (&head, head, head->tsv->tag);
      data->value = head;
      data->n[0]--;
            mitkIpPicFreeTag (tag);
        }
        /* build and store the level */
        tag = (mitkIpPicTSV_t *) malloc (sizeof (mitkIpPicTSV_t));
        if (!tag) {
            ipMITKSegmentationError (ipMITKSegmentationOUT_OF_MEMORY);
        }
        strcpy (tag->tag, "IMAGE");
        tag->type = segmentation->type;
        tag->bpe = segmentation->bpe;
        tag->dim = segmentation->dim;
        tag->n[0] = segmentation->n[0];
        tag->n[1] = segmentation->n[1];
        tag->value = malloc (_mitkIpPicSize (segmentation));
        memcpy (tag->value, segmentation->data, _mitkIpPicSize (segmentation));
        mitkIpPicAddSubTag (data, tag);
    }
}

void
ipMITKSegmentationUndo (mitkIpPicDescriptor* segmentation)
{
    mitkIpPicTSV_t *undo, *data;

    assert (segmentation);
    undo = mitkIpPicQueryTag (segmentation, tagUNDO);
    if (!undo) {
        ipMITKSegmentationError (ipMITKSegmentationUNDO_DISABLED);
    }

    /* if any level is stored ... */
    data = mitkIpPicQuerySubTag (undo, tagUNDO_DATA);
    if (data->n[0]) {
        /* ... replace the image data and remove this level */
        _mitkIpPicTagsElement_t* head = (_mitkIpPicTagsElement_t *) data->value;
        _mitkIpPicTagsElement_t* current = head;
        mitkIpPicTSV_t* tag;

        while( current->next != NULL ) {
            current = current->next;
        }
        tag = _mitkIpPicRemoveTag (&head, current, current->tsv->tag);
  data->value = head;
  data->n[0]--;
        memcpy (segmentation->data, tag->value, _mitkIpPicSize (segmentation));
        mitkIpPicFreeTag (tag);

  tag = mitkIpPicDelTag (segmentation, tagSEGMENTATION_EMPTY);
  if (tag) {
    mitkIpPicFreeTag (tag);
  }
    }
}

ipBool_t
ipMITKSegmentationUndoAvailable (mitkIpPicDescriptor* segmentation)
{
    mitkIpPicTSV_t *undo, *data;

    assert (segmentation);
    undo = mitkIpPicQueryTag (segmentation, tagUNDO);
    if (!undo) {
        ipMITKSegmentationError (ipMITKSegmentationUNDO_DISABLED);
    }
    data = mitkIpPicQuerySubTag (undo, tagUNDO_DATA);
    return (data->n[0] ? ipTrue : ipFalse);
}

mitkIpPicTSV_t *
_mitkIpPicRemoveTag( _mitkIpPicTagsElement_t **head, _mitkIpPicTagsElement_t *which, char *tag)
{
  mitkIpPicTSV_t *tsv = NULL;

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
