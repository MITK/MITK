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

#include <ipPic/ipPicTypeMultiplex.h>
#include "ipSegmentation.h"



// only make macros available in local scope:
namespace {

// returns true if segmentation pixel at ofs is set, includes check for border pixels
#define SEGSET(ofs) (              \
((ofs)>=0 && (ofs)<maxOfs) &&          \
( *(((PicType*)seg->data) + (ofs)) != 0 ) &&  \
( (ofs)%line != 0 || (pos+1)%line != 0 ) &&    \
( (ofs+1)%line != 0 || (pos)%line != 0 )      \
)


// appends a point to result, sets finished to true if this point is the same as the first one
// allocates new momory if necessary
#define ADD_CONTOUR_POINT            \
result[numPts*2]   = xPos+xCorner[dir];      \
result[numPts*2+1] = yPos+yCorner[dir];      \
if (result[numPts*2]==result[0] && result[numPts*2+1]==result[1] && numPts>0) finished = true; \
numPts++;                    \
if (numPts==resSize) {              \
  resSize+=16+resSize/2;            \
  result = (float*)realloc( result, resSize*2*sizeof(float) ); \
  if (!result) finished = true;        \
}

}

template<typename PicType>
float* tmGetContour4N( const ipPicDescriptor *seg, int startOfs, int &numPts, int &resSize, float *result )
// returns the number of contour points (xy-pairs) in result
// optimized macros: DON'T TOUCH THE CODE ;-)
{
  numPts = 0; 
  int line   = seg->n[0];
  int maxOfs = seg->n[0] * seg->n[1];
  
  int straight[4]  = { 1, -line, -1, line };
  int right[4]  = { line, 1, -line, -1 };
  float xMod[4]    = { 1.0, 0.0, -1.0, 0.0 };
  float yMod[4]    = { 0.0, -1.0, 0.0, 1.0 };
  float xCorner[4]  = { 1.0, 1.0, 0.0, 0.0 };
  float yCorner[4]  = { 1.0, 0.0, 0.0, 1.0 };
  
  int dir = 0;
  int pos = startOfs;
  float xPos = (float)(pos % line);
  float yPos = (float)(pos / line);

  while ( SEGSET( pos+right[dir] ) && dir<4 ) dir++;
  if (dir==4) return result;  // no contour pixel
  
  bool finished = false;
  if (result==0) {
    resSize = 2048;
    result = (float*)malloc( resSize*2*sizeof(float) );
  }

  do {
    if ( SEGSET( pos+right[dir] ) ) {
      // modify direction (turn right):
      dir = (dir-1) & 3;
      // modify position:
      pos  += straight[dir];
      xPos += xMod[dir];
      yPos += yMod[dir];
    }
    else if ( SEGSET( pos+straight[dir] ) ) {
      ADD_CONTOUR_POINT
      // modify position:
      pos += straight[dir];
      xPos += xMod[dir];
      yPos += yMod[dir];
    }
    else {
      ADD_CONTOUR_POINT
      // modify direction (turn left):
      dir = (dir+1) & 3;
    }
  } while (!finished);
  return result;
}


float* ipMITKSegmentationGetContour4N( const ipPicDescriptor *seg, int startOfs, int &numPoints, int &sizeBuffer, float *pointBuffer )
{
  float *newBuffer;
  ipPicTypeMultiplexR4( tmGetContour4N, seg, newBuffer, startOfs, numPoints, sizeBuffer, pointBuffer );
  return newBuffer;
}


template<typename PicType>
float* tmGetContour8N( const ipPicDescriptor *seg, int startOfs, int &numPts, int &resSize, float *result )
// returns the number of contour points (xy-pairs) in result
// optimized macros: DON'T TOUCH THE CODE ;-)
{
  numPts = 0; 
  int line   = seg->n[0];
  int maxOfs = seg->n[0] * seg->n[1];
  
  int straight[4]  = { 1, -line, -1, line };
  int right[4]  = { line, 1, -line, -1 };
  float xMod[4]    = { 1.0, 0.0, -1.0, 0.0 };
  float yMod[4]    = { 0.0, -1.0, 0.0, 1.0 };
  float xCorner[4]  = { 1.0, 1.0, 0.0, 0.0 };
  float yCorner[4]  = { 1.0, 0.0, 0.0, 1.0 };
  
  int dir = 0;
  int pos = startOfs;
  float xPos = (float)(pos % line);
  float yPos = (float)(pos / line);

  while ( SEGSET( pos+right[dir] ) && dir<4 ) dir++;
  if (dir==4) {
    // check diagonal pixels:
    dir = 0;
    while ( SEGSET( pos+right[dir]+straight[dir] ) && dir<4 ) dir++;
    if (dir==4) return result;  // no contour pixel
    // chose next suitable neighbour:
    pos  += straight[dir];
    xPos += xMod[dir];
    yPos += yMod[dir];
  }
  
  bool finished = false;
  if (result==0) {
    resSize = 2048;
    result = (float*)malloc( resSize*2*sizeof(float) );
  }

  do {
    if ( SEGSET( pos+right[dir] ) ) {      // valgrind complaint: jump dependent on uninitialized value
      // modify direction (turn right):
      dir = (dir-1) & 3;
      // modify position:
      pos  += straight[dir];
      xPos += xMod[dir];
      yPos += yMod[dir];
    }
    else if ( SEGSET( pos+straight[dir] ) ) {
      ADD_CONTOUR_POINT
      // modify position:
      pos += straight[dir];
      xPos += xMod[dir];
      yPos += yMod[dir];
    }
    else if ( SEGSET( pos+right[dir]+straight[dir] ) ) {   // valgrind complaint: jump dependent on uninitialized value
      ADD_CONTOUR_POINT
      // modify position:
      pos += straight[dir];
      xPos += xMod[dir];
      yPos += yMod[dir];
      // modify direction (turn right):
      dir = (dir-1) & 3;
      // modify position second time:
      pos += straight[dir];
      xPos += xMod[dir];
      yPos += yMod[dir];
    }
    else {
      ADD_CONTOUR_POINT
      // modify direction (turn left):
      dir = (dir+1) & 3;
    }
  } while (!finished);
  return result;
}


float* ipMITKSegmentationGetContour8N( const ipPicDescriptor *seg, int startOfs, int &numPoints, int &sizeBuffer, float *pointBuffer )
{
  float *newBuffer;
  ipPicTypeMultiplexR4( tmGetContour8N, seg, newBuffer, startOfs, numPoints, sizeBuffer, pointBuffer );
  return newBuffer;
}
