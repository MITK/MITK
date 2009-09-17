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

#include <queue>
#include "ipSegmentation.h"

int
ipMITKSegmentationReplaceRegion4N( mitkIpPicDescriptor *seg, int startOfs, mitkIpInt1_t newValue )
{
  mitkIpPicTSV_t* tag;

  std::queue<int> ofsQueue;

  if (ipMITKSegmentationUndoIsEnabled (seg)) {
    ipMITKSegmentationUndoSave (seg);
  }
  tag = mitkIpPicDelTag (seg, tagSEGMENTATION_EMPTY);
  if (tag) {
    mitkIpPicFreeTag (tag);
  }

  if (seg->bpe != 8) return 0;

  int line = seg->n[0];
  int maxOfs = (int)(line * seg->n[1]);
  int testOfs;
  mitkIpInt1_t replaceMe = *((mitkIpInt1_t*)seg->data + startOfs);
  if (replaceMe == newValue) return 0;
  
  mitkIpInt1_t segVal;
  ofsQueue.push( startOfs );
  *((mitkIpInt1_t*)seg->data+startOfs) = newValue;
  int regionSize = 0;

  while (!ofsQueue.empty()) {
    int nextOfs = ofsQueue.front();
    ofsQueue.pop();
    regionSize++;
    // check right:
    testOfs = nextOfs+1;
    if (testOfs%line!=0) {
      segVal = *((mitkIpInt1_t*)seg->data+testOfs);
      if ( segVal == replaceMe ) {
        ofsQueue.push( testOfs );
        *((mitkIpInt1_t*)seg->data+testOfs) = newValue;
      }
    }
    // check top:
    testOfs = nextOfs-line;
    if (testOfs >= 0) {
      segVal = *((mitkIpInt1_t*)seg->data+testOfs);
      if ( segVal == replaceMe ) {
        ofsQueue.push( testOfs );
        *((mitkIpInt1_t*)seg->data+testOfs) = newValue;
      }
    }
    // check left:
    testOfs = nextOfs-1;
    if (nextOfs%line!=0) {
      segVal = *((mitkIpInt1_t*)seg->data+testOfs);
      if ( segVal == replaceMe ) {
        ofsQueue.push( testOfs );
        *((mitkIpInt1_t*)seg->data+testOfs) = newValue;
      }
    }
    // check bottom:
    testOfs = nextOfs+line;
    if (testOfs < maxOfs) {
      segVal = *((mitkIpInt1_t*)seg->data+testOfs);
      if ( segVal == replaceMe ) {
        ofsQueue.push( testOfs );
        *((mitkIpInt1_t*)seg->data+testOfs) = newValue;
      }
    }
  }
  return regionSize;
}
