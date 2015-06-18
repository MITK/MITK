/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <queue>
#include "ipSegmentation.h"


mitkIpPicDescriptor*
ipMITKSegmentationCreateGrowerHistory( mitkIpPicDescriptor *seg, int startOfs, mitkIpPicDescriptor *histBuffer )
{
  std::queue<int> ofsQueue;

  if (!seg) return 0;
  if (!histBuffer) {
    histBuffer = mitkIpPicCopyHeader( seg, histBuffer );
    histBuffer->type = mitkIpPicUInt;
    histBuffer->bpe = 16;
    mitkIpUInt4_t size = _mitkIpPicSize( histBuffer );
    histBuffer->data = malloc( size );
    memset( histBuffer->data, 0, size );  // clear buffer
  }
  else {
    // check if dimension of histBuffer is valid, if not: change it!
    if (histBuffer->n[0] != seg->n[0] || histBuffer->n[1] != seg->n[1]) {
      histBuffer->n[0] = seg->n[0];
      histBuffer->n[1] = seg->n[1];
      mitkIpUInt4_t size = _mitkIpPicSize( histBuffer );
      histBuffer->data = realloc( histBuffer->data, size );
      if (histBuffer->data == 0) return 0;
      memset( histBuffer->data, 0, size );  // clear buffer
    }
  }

  // create a clear buffer to check wether a point has already been visited
  // (seg cannot be modifier and histBuffer can contain any value)
  mitkIpPicDescriptor *flagBuffer = mitkIpPicCopyHeader( seg, 0 );
  mitkIpUInt4_t size = _mitkIpPicSize( flagBuffer );
  flagBuffer->data = malloc( size );
  memset( flagBuffer->data, 0, size );
  *((mitkIpUInt1_t*)flagBuffer->data+startOfs) = 1;    // flag pixel as visited

  int line = seg->n[0];
  int maxOfs = (int)(line * seg->n[1]);
  int testOfs;
  mitkIpUInt1_t segVal, flagVal;
  int iteration = 0;
  int currentWave = 1;
  int nextWave = 0;

  ofsQueue.push( startOfs );

  while (!ofsQueue.empty()) {
    int nextOfs = ofsQueue.front();
    ofsQueue.pop();
    currentWave--;
    *((mitkIpUInt2_t*)histBuffer->data+nextOfs) = (mitkIpUInt2_t)(iteration+1);  // seed point should get history = 1

    // check right:
    testOfs = nextOfs+1;
    if (testOfs%line!=0) {
      segVal = *((mitkIpUInt1_t*)seg->data+testOfs);
      flagVal = *((mitkIpUInt1_t*)flagBuffer->data+testOfs);
      if ( segVal != 0 && flagVal == 0) {
        ofsQueue.push( testOfs );
        *((mitkIpUInt1_t*)flagBuffer->data+testOfs) = 1;    // flag pixel as visited
        nextWave++;
      }
    }
    // check top:
    testOfs = nextOfs-line;
    if (testOfs > 0) {
      segVal = *((mitkIpUInt1_t*)seg->data+testOfs);
      flagVal = *((mitkIpUInt1_t*)flagBuffer->data+testOfs);
      if ( segVal != 0 && flagVal == 0) {
        ofsQueue.push( testOfs );
        *((mitkIpUInt1_t*)flagBuffer->data+testOfs) = 1;    // flag pixel as visited
        nextWave++;
      }
    }
    // check left:
    testOfs = nextOfs-1;
    if (nextOfs%line!=0) {
      segVal = *((mitkIpUInt1_t*)seg->data+testOfs);
      flagVal = *((mitkIpUInt1_t*)flagBuffer->data+testOfs);
      if ( segVal != 0 && flagVal == 0) {
        ofsQueue.push( testOfs );
        *((mitkIpUInt1_t*)flagBuffer->data+testOfs) = 1;    // flag pixel as visited
        nextWave++;
      }
    }
    // check bottom:
    testOfs = nextOfs+line;
    if (testOfs < maxOfs) {
      segVal = *((mitkIpUInt1_t*)seg->data+testOfs);
      flagVal = *((mitkIpUInt1_t*)flagBuffer->data+testOfs);
      if ( segVal != 0 && flagVal == 0) {
        ofsQueue.push( testOfs );
        *((mitkIpUInt1_t*)flagBuffer->data+testOfs) = 1;    // flag pixel as visited
        nextWave++;
      }
    }
    // check for number of iterations:
    if (currentWave == 0) {
      currentWave = nextWave;
      nextWave = 0;
      iteration++;
    }
  }

  mitkIpPicFree( flagBuffer );
  return histBuffer;
}
