/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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
#include <queue>
#include <ipPic/mitkIpPicTypeMultiplex.h>
#include "ipSegmentation.h"

/*  
Starts a 4 neighbourhood region growing at startOfs (y*picWidth+x) of the 2D image src.
If relativeBounds is true, the region grows in [base-lowerBound, base+upperBound], in which base is the average
color of the 9 pixels around startOfs. If relativeBounds is false, the region grows in [lowerBound, upperBound].
If maxIterations is > 0, the growing process is stopped after maxIterations.
If segBuffer is 0, new memory for the segmented image is allocated and returned, else the segBuffer is used
to store the result (has to be an 8-bit datatype, e.g. mitkIpUInt1_t).
histBuffer must be 0 or a pointer to a 16-bit mitkIpUInt1_t image of the same size as src. In case of the latter, 
history data is written to that buffer: the seed pixel gets a 1, all direct neighbours 2 etc. The buffer is
not cleared in this function and can thus hold history data of several growing processes in different areas.
*/
template<typename PicType>
mitkIpPicDescriptor*
tmGrowRegion4N( mitkIpPicDescriptor *src, int startOfs, bool relativeBounds, float lowerBoundFlt, float upperBoundFlt, int maxIterations, mitkIpPicDescriptor *segBuffer, int &contourOfs, float &startCol, mitkIpPicDescriptor *histBuffer )
{
  PicType lowerBound = static_cast<PicType>(lowerBoundFlt);
  PicType upperBound = static_cast<PicType>(upperBoundFlt);
  std::queue<int> ofsQueue;

  if (maxIterations <= 0) maxIterations = 32000;
  if (!src) return 0;
  if (!segBuffer) {
    segBuffer = mitkIpPicCopyHeader( src, segBuffer );
    segBuffer->type = mitkIpPicUInt;
    segBuffer->bpe = 8; 
    mitkIpUInt4_t size = _mitkIpPicSize( segBuffer );
    segBuffer->data = malloc( size );
  }
  else {
    // check if dimension of segBuffer is valid, if not: change it!
    if (segBuffer->n[0] != src->n[0] || segBuffer->n[1] != src->n[1]) {
      segBuffer->n[0] = src->n[0];
      segBuffer->n[1] = src->n[1];
      mitkIpUInt4_t size = _mitkIpPicSize( segBuffer );
      segBuffer->data = realloc( segBuffer->data, size );
      if (segBuffer->data == 0) return 0;
    }
  }
  if (histBuffer) {
    // check if dimension of histBuffer is valid, if not: change it!
    if (histBuffer->n[0] != src->n[0] || histBuffer->n[1] != src->n[1]) {
      histBuffer->n[0] = src->n[0];
      histBuffer->n[1] = src->n[1];
      mitkIpUInt4_t size = _mitkIpPicSize( histBuffer );
      histBuffer->data = realloc( histBuffer->data, size );
      if (histBuffer->data == 0) return 0;
      memset( histBuffer->data, 0, size );  // clear buffer
    }
  }
  
  int line = segBuffer->n[0];
  int maxOfs = (int)(line * segBuffer->n[1]);
  //PicType *start = ((PicType*)src->data) + startOfs;
  // init borders:
  PicType lowest, highest;
  if (relativeBounds) {
    
    // average base color from 3x3 block:
    // check for edges of image
    int offset;
    int numberOfValidOffsets = 0;
    int baseCol = 0;
    offset = startOfs;           if ( (offset >= 0) && (offset < (int)(src->n[0] * src->n[1])) ) { baseCol += *((PicType*)(src->data)+offset); ++numberOfValidOffsets; }
    offset = startOfs+1;         if ( (offset >= 0) && (offset < (int)(src->n[0] * src->n[1])) ) { baseCol += *((PicType*)(src->data)+offset); ++numberOfValidOffsets; }
    offset = startOfs+1-line;    if ( (offset >= 0) && (offset < (int)(src->n[0] * src->n[1])) ) { baseCol += *((PicType*)(src->data)+offset); ++numberOfValidOffsets; }
    offset = startOfs-line;      if ( (offset >= 0) && (offset < (int)(src->n[0] * src->n[1])) ) { baseCol += *((PicType*)(src->data)+offset); ++numberOfValidOffsets; }
    offset = startOfs-1-line;    if ( (offset >= 0) && (offset < (int)(src->n[0] * src->n[1])) ) { baseCol += *((PicType*)(src->data)+offset); ++numberOfValidOffsets; }
    offset = startOfs-1;         if ( (offset >= 0) && (offset < (int)(src->n[0] * src->n[1])) ) { baseCol += *((PicType*)(src->data)+offset); ++numberOfValidOffsets; }
    offset = startOfs-1+line;    if ( (offset >= 0) && (offset < (int)(src->n[0] * src->n[1])) ) { baseCol += *((PicType*)(src->data)+offset); ++numberOfValidOffsets; }
    offset = startOfs+line;      if ( (offset >= 0) && (offset < (int)(src->n[0] * src->n[1])) ) { baseCol += *((PicType*)(src->data)+offset); ++numberOfValidOffsets; }
    offset = startOfs+1+line;    if ( (offset >= 0) && (offset < (int)(src->n[0] * src->n[1])) ) { baseCol += *((PicType*)(src->data)+offset); ++numberOfValidOffsets; }

    if ( numberOfValidOffsets > 0 )
      baseCol = (PicType)( (float)baseCol / (float)numberOfValidOffsets );

    lowest = baseCol - lowerBound;
    highest = baseCol + upperBound;
    startCol = (float)baseCol;
  }
  else {
    lowest = lowerBound;
    highest = upperBound;
    startCol = 0.0f;
  }

  memset( segBuffer->data, 0, _mitkIpPicSize(segBuffer) );  // clear buffer
    
  PicType value = *((PicType*)src->data+startOfs);
  if ( value >=lowest && value <=highest ) {
    ofsQueue.push( startOfs );
  }

  contourOfs = -1;
  int testOfs;
  mitkIpUInt1_t segVal;
  int iteration = 0;
  int currentWave = 1;
  int nextWave = 0;

  while (!ofsQueue.empty() && iteration<=maxIterations) {
    int nextOfs = ofsQueue.front();
    ofsQueue.pop();
    currentWave--;
    *((mitkIpUInt1_t*)segBuffer->data+nextOfs) = 1;
    if (histBuffer) {
      *((mitkIpUInt2_t*)histBuffer->data+nextOfs) = (mitkIpUInt2_t)(iteration+1);  // seed point should get history = 1
    }
    if (nextOfs > contourOfs) contourOfs = nextOfs;
    // check right:
    testOfs = nextOfs+1;
    if (testOfs%line!=0) {
      segVal = *((mitkIpUInt1_t*)segBuffer->data+testOfs);
      if ( segVal == 0 ) {
        value = *((PicType*)src->data+testOfs);
        if ( value >=lowest && value <=highest ) {
          ofsQueue.push( testOfs );
          nextWave++;
          *((mitkIpUInt1_t*)segBuffer->data+testOfs) = 2;
        }
      }
    }
    // check top:
    testOfs = nextOfs-line;
    if (testOfs > 0) {
      segVal = *((mitkIpUInt1_t*)segBuffer->data+testOfs);
      if ( segVal == 0 ) {
        value = *((PicType*)src->data+testOfs);
        if ( value >=lowest && value <=highest ) {
          ofsQueue.push( testOfs );
          nextWave++;
          *((mitkIpUInt1_t*)segBuffer->data+testOfs) = 2;
        }
      }
    }
    // check left:
    testOfs = nextOfs-1;
    if (nextOfs%line!=0) {
      segVal = *((mitkIpUInt1_t*)segBuffer->data+testOfs);
      if ( segVal == 0 ) {
        value = *((PicType*)src->data+testOfs);
        if ( value >=lowest && value <=highest ) {
          ofsQueue.push( testOfs );
          nextWave++;
          *((mitkIpUInt1_t*)segBuffer->data+testOfs) = 2;
        }
      }
    }
    // check bottom:
    testOfs = nextOfs+line;
    if (testOfs < maxOfs) {
      segVal = *((mitkIpUInt1_t*)segBuffer->data+testOfs);
      if ( segVal == 0 ) {
        value = *((PicType*)src->data+testOfs);
        if ( value >=lowest && value <=highest ) {
          ofsQueue.push( testOfs );
          nextWave++;
          *((mitkIpUInt1_t*)segBuffer->data+testOfs) = 2;
        }
      }
    }
    // check for number of iterations:
    if (currentWave == 0) {
      currentWave = nextWave;
      nextWave = 0;
      iteration++;
    }
  }

  return segBuffer;
}


mitkIpPicDescriptor*
ipMITKSegmentationGrowRegion4N( mitkIpPicDescriptor *src, int startOfs, bool relativeBounds, float lowerBound, float upperBound, int maxIterations, mitkIpPicDescriptor *segBuffer, mitkIpPicDescriptor *histBuffer )
{
  mitkIpPicDescriptor *result = 0;
  int contourOfs;
  float startCol;
  
  if (ipMITKSegmentationUndoIsEnabled (segBuffer)) {
    ipMITKSegmentationUndoSave (segBuffer);
  }

  mitkIpPicTypeMultiplexR9( tmGrowRegion4N, src, result, startOfs, relativeBounds, lowerBound, upperBound, maxIterations, segBuffer, contourOfs, startCol, histBuffer );
  return result;
}



mitkIpPicDescriptor*
ipMITKSegmentationGrowRegion4N( mitkIpPicDescriptor *src, int startOfs, bool relativeBounds, float lowerBound, float upperBound, int maxIterations, mitkIpPicDescriptor *segBuffer, int &contourOfs, float &startCol, mitkIpPicDescriptor *histBuffer )
{
  mitkIpPicDescriptor *result = 0;

  if (ipMITKSegmentationUndoIsEnabled (segBuffer)) {
    ipMITKSegmentationUndoSave (segBuffer);
  }

  mitkIpPicTypeMultiplexR9( tmGrowRegion4N, src, result, startOfs, relativeBounds, lowerBound, upperBound, maxIterations, segBuffer, contourOfs, startCol, histBuffer );
  return result;
}
