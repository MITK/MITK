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
ipMITKSegmentationReplaceRegion4N( ipPicDescriptor *seg, int startOfs, ipInt1_t newValue )
{
  ipPicTSV_t* tag;

  std::queue<int> ofsQueue;

  if (ipMITKSegmentationUndoIsEnabled (seg)) {
    ipMITKSegmentationUndoSave (seg);
  }
  tag = ipPicDelTag (seg, (char*)tagSEGMENTATION_EMPTY);
  if (tag) {
    ipPicFreeTag (tag);
  }

  if (seg->bpe != 8) return 0;

  int line = seg->n[0];
  int maxOfs = (int)(line * seg->n[1]);
  int testOfs;
  ipInt1_t replaceMe = *((ipInt1_t*)seg->data + startOfs);
  if (replaceMe == newValue) return 0;
  
  ipInt1_t segVal;
  ofsQueue.push( startOfs );
  *((ipInt1_t*)seg->data+startOfs) = newValue;
  int regionSize = 0;

  while (!ofsQueue.empty()) {
    int nextOfs = ofsQueue.front();
    ofsQueue.pop();
    regionSize++;
    // check right:
    testOfs = nextOfs+1;
    if (testOfs%line!=0) {
      segVal = *((ipInt1_t*)seg->data+testOfs);
      if ( segVal == replaceMe ) {
        ofsQueue.push( testOfs );
        *((ipInt1_t*)seg->data+testOfs) = newValue;
      }
    }
    // check top:
    testOfs = nextOfs-line;
    if (testOfs >= 0) {
      segVal = *((ipInt1_t*)seg->data+testOfs);
      if ( segVal == replaceMe ) {
        ofsQueue.push( testOfs );
        *((ipInt1_t*)seg->data+testOfs) = newValue;
      }
    }
    // check left:
    testOfs = nextOfs-1;
    if (nextOfs%line!=0) {
      segVal = *((ipInt1_t*)seg->data+testOfs);
      if ( segVal == replaceMe ) {
        ofsQueue.push( testOfs );
        *((ipInt1_t*)seg->data+testOfs) = newValue;
      }
    }
    // check bottom:
    testOfs = nextOfs+line;
    if (testOfs < maxOfs) {
      segVal = *((ipInt1_t*)seg->data+testOfs);
      if ( segVal == replaceMe ) {
        ofsQueue.push( testOfs );
        *((ipInt1_t*)seg->data+testOfs) = newValue;
      }
    }
  }
  return regionSize;
}
