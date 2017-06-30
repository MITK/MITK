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

#include <mitkIpPicTypeMultiplex.h>
#include <ANN/ANN.h>
#include "ipSegmentation.h"



static ANNkd_tree *annTree;
static ANNpoint queryPt;
static ANNidxArray nnIdx;
static ANNdistArray dists;



// find next pixel in ANN:
#define QUERY_DIST(qOfs)                  \
  queryPt[0] = (float)(qOfs % line) + 0.5;        \
    queryPt[1] = (float)(qOfs / line) + 0.5;        \
    annTree->annkSearch( queryPt, 1, nnIdx, dists );


#define PROCESS_PIXEL                        \
  histVal = *((mitkIpUInt2_t*)history->data+testOfs);          \
  segVal = *((mitkIpUInt1_t*)seg->data+testOfs);            \
  if ( segVal > 0 && histVal <= maxHist && testOfs!=oldOfs ) {  \
    grad = ipMITKSegmentationGetDistGradient( testOfs, seg );    \
    QUERY_DIST(testOfs)                      \
    if (grad<minGrad) {                      \
      minGrad = grad;                      \
      gradCand = testOfs;                    \
    }                              \
    if (dists[0] > maxDist) {                  \
      maxDist = dists[0];                    \
      candOfs = testOfs;                    \
    }                              \
  }


float ipMITKSegmentationGetDistGradient( int ofs, mitkIpPicDescriptor *seg )
{
  mitkIpUInt4_t unsignedOfs = ofs < 0 ? seg->n[0]*seg->n[1] - seg->n[0] : ofs;
  if (unsignedOfs < seg->n[0] || unsignedOfs >= seg->n[0]*seg->n[1] - seg->n[0]) // exclude image borders
  {
    return 1.0; // initialization value of minGrad (high gradient)
  }
  float x = (float)(ofs % seg->n[0]) + 0.5;
        float y = (float)(ofs / seg->n[0]) + 0.5;
  mitkIpUInt1_t segVal = 0; // initialize to stop valgrind's warning about "Conditional jump or move depends on uninitialised value(s)"

  queryPt[0] = x+1;    queryPt[1] = y;
    annTree->annkSearch( queryPt, 1, nnIdx, dists );
  float d1 = sqrt( dists[0] );  // right dist
  segVal = *((mitkIpUInt1_t*)seg->data+ofs+1);
  if (!segVal) d1 = -10.0;
  queryPt[0] = x-1;    queryPt[1] = y;
    annTree->annkSearch( queryPt, 1, nnIdx, dists );
  float d2 = sqrt( dists[0] );  // left dist
  segVal = *((mitkIpUInt1_t*)seg->data+ofs-1);
  if (!segVal) d2 = -10.0;
  queryPt[0] = x;      queryPt[1] = y+1;
    annTree->annkSearch( queryPt, 1, nnIdx, dists );
  float d3 = sqrt( dists[0] );  // lower dist
  segVal = *((mitkIpUInt1_t*)seg->data+ofs+seg->n[0]);
  if (!segVal) d3 = -10.0;
  queryPt[0] = x;      queryPt[1] = y-1;
    annTree->annkSearch( queryPt, 1, nnIdx, dists );
  float d4 = sqrt( dists[0] );  // upper dist
  segVal = *((mitkIpUInt1_t*)seg->data+ofs-seg->n[0]);
  if (!segVal) d4 = -10.0;
  float res = 0.5*(float)sqrt( (d1-d2)*(d1-d2) + (d3-d4)*(d3-d4) );
  return res;
}



tCutResult ipMITKSegmentationGetCutPoints( mitkIpPicDescriptor *seg, mitkIpPicDescriptor *history, int ofs )
{
  bool debug(false);
  tCutResult res;
  int resContourSize = 5000;
  res.traceline = (float*)malloc( resContourSize*sizeof(float)*2 );
  res.onGradient = (bool*)malloc( resContourSize*sizeof(bool) );
  res.numPoints = 0;
  res.absMin = 0;
  res.cutIt = false;
  res.deleteCurve = 0;

  if (!history) return res;                  // no history!
  if (*((mitkIpUInt2_t*)history->data + ofs) == 0) return res;  // ofs not inside known history

  // get one point on the contour:
  mitkIpUInt1_t *ptr = (mitkIpUInt1_t*)seg->data + ofs;
    int endLine = ((ofs / seg->n[0]) + 1) * seg->n[0] -1;
  int contourOfs = ofs;
  while (contourOfs!=endLine && *ptr!=0) {
    ptr++;
    contourOfs++;
  }
  if (*ptr == 0) contourOfs--;  // get back on the contour!

  // extract the contour:
  int sizeContour, sizeBuffer;
  float *contour = ipMITKSegmentationGetContour8N( seg, contourOfs, sizeContour, sizeBuffer );
  // init ANN tree with contour points:
  queryPt    = annAllocPt( 2 );
    ANNpointArray dataPts  = annAllocPts( sizeContour, 2 );
    nnIdx    = new ANNidx[10];
    dists    = new ANNdist[10];
    for (int i=0; i<sizeContour; i++) {
        (dataPts[i])[0] = contour[2*i];
        (dataPts[i])[1] = contour[2*i+1];
    }
    annTree = new ANNkd_tree( dataPts, sizeContour, 2 );

  // trace to center:
  int line = history->n[0];

  int maxOfs = line * history->n[1];
  QUERY_DIST(ofs)
  float maxDist = dists[0];
  float minDist = 10000;  // current minimum distance from border
  float oldDist = 0;
  int candOfs = ofs;
  int nextOfs = -1;
  int oldOfs, testOfs, gradCand=-1;
  float grad, minGrad;
  bool skelettonReached = false;
  mitkIpUInt2_t histVal;
  mitkIpUInt1_t segVal;
  mitkIpUInt2_t maxHist = 10000;
  if (maxHist==0 && debug) printf( "maxHist = 0!\n" );
  do {
    oldOfs = nextOfs;
    nextOfs = candOfs;
    // store point info:
    if (res.numPoints < resContourSize) {
      res.traceline[2*res.numPoints] = (float)(nextOfs % line) + 0.5;
      res.traceline[2*res.numPoints+1] = (float)(nextOfs / line) + 0.5;
      if (nextOfs==gradCand) res.onGradient[res.numPoints] = true;
      else res.onGradient[res.numPoints] = false;

      if (debug)
      {
        printf( "(%.f,%.f): H=%i, G=%i\n", res.traceline[2*res.numPoints],
        res.traceline[2*res.numPoints+1],
        *((mitkIpUInt2_t*)history->data+nextOfs),
        res.onGradient[res.numPoints] );
      }
      res.numPoints++;

      if (res.numPoints == resContourSize)
      {
        resContourSize *= 2; // explodes, but such contours must be very strange
        res.traceline = (float*)realloc( res.traceline,  resContourSize*sizeof(float)*2 );
        res.onGradient = (bool*)realloc( res.onGradient, resContourSize*sizeof(bool) );
        if ((res.traceline == nullptr) || (res.onGradient == nullptr))
        {
          res.numPoints = 0;
          res.cutIt = false;
          return res;
        }
      }
    }

    maxHist = *((mitkIpUInt2_t*)history->data + nextOfs);  // don't exceed this history!
    maxDist = 0;  // clear maxDist
    minGrad = 1.0;  // clear minGrad

    int traceSinceMin = res.numPoints - 1 - res.absMin;
    float weight = 20.0 / (20.0+traceSinceMin);
    if (weight < 0.5) weight = 0.5;
    QUERY_DIST(nextOfs)
    if (!skelettonReached && dists[0] < oldDist) {
      skelettonReached = true;
                        if (debug) printf( "skeletton reached at %i, oldDist=%.1f, currentDist=%.1f\n",
        res.numPoints - 1, oldDist, dists[0] );
    }
    oldDist = dists[0];
    if (skelettonReached && weight*dists[0] < minDist) {
      minDist = dists[0];
      res.absMin = res.numPoints - 1;  // has already been increased
    }

    // check right:
    testOfs = nextOfs+1;
    if (testOfs%line!=0) {
      // check top right:
      PROCESS_PIXEL
      testOfs = nextOfs-line;
      if (testOfs > 0) {
        testOfs++;
        PROCESS_PIXEL
      }
      // check bottom right:
      testOfs = nextOfs+line;
      if (testOfs < maxOfs) {
        testOfs++;
        PROCESS_PIXEL
      }
    }
    // check top:
    testOfs = nextOfs-line;
    if (testOfs > 0) {
      PROCESS_PIXEL
    }
    // check left:
    testOfs = nextOfs-1;
    if (nextOfs%line!=0) {
      PROCESS_PIXEL
      // check top left:
      testOfs = nextOfs-line;
      if (testOfs > 0) {
        testOfs--;
        PROCESS_PIXEL
      }
      // check bottom left:
      testOfs = nextOfs+line;
      if (testOfs < maxOfs) {
        testOfs--;
        PROCESS_PIXEL
      }
    }
    // check bottom:
    testOfs = nextOfs+line;
    if (testOfs < maxOfs) {
      PROCESS_PIXEL
    }
    // check for run on gradient:
    if (minGrad < 0.5) {
      candOfs = gradCand;
                        if (debug) printf( "." );
    }
    else if (debug) printf( "x" );
  } while (candOfs != nextOfs && maxHist > 0);

  if (res.absMin < (res.numPoints-10)) {
    res.absMin += (int)(sqrt(minDist)/2.0);
//  int cutX = (int)(res.traceline[2*res.absMin]-0.5);
//  int cutY = (int)(res.traceline[2*res.absMin+1]-0.5);
//  int cutOfs = cutX + line*cutY;
//  histVal = *((mitkIpUInt2_t*)history->data+cutOfs);
//  printf( "histVal at Cut=%i\n", histVal );
//  if (histVal > 1) {
    res.cutIt = true;
    float cutXf = (float)res.traceline[2*res.absMin];
    float cutYf = (float)res.traceline[2*res.absMin+1];
    queryPt[0] = cutXf;
    queryPt[1] = cutYf;
    annTree->annkSearch( queryPt, 1, nnIdx, dists );
    int cutIdx1 = nnIdx[0];
    res.cutCoords[0] = contour[2*cutIdx1];
    res.cutCoords[1] = contour[2*cutIdx1+1];
    int cutIdx2 = cutIdx1;
    float minDist = 100000000;
    int testCnt = 0;
    for (int i=0; i<sizeContour; i++) {
      int idxDif = abs(cutIdx1-i);
      // take wraparound into account:
      if (idxDif > (sizeContour/2)) idxDif = sizeContour - idxDif;
      if ( idxDif > 50 ) {
        float dist = (cutXf-contour[2*i])*(cutXf-contour[2*i]) + (cutYf-contour[2*i+1])*(cutYf-contour[2*i+1]);
        if (dist < minDist) {
          minDist = dist;
          cutIdx2 = i;
        }
      }
      else testCnt++;
    }
    res.cutCoords[2] = contour[2*cutIdx2];
    res.cutCoords[3] = contour[2*cutIdx2+1];
    if (debug) printf( "idx1=%i, idx2=%i, %i pts not evaluated.\n", cutIdx1, cutIdx2, testCnt );

                if ((res.cutCoords[0] == res.cutCoords[2]) &&
                    (res.cutCoords[1] == res.cutCoords[3]))
                {
                  free( contour );
                  // free ANN stuff:
                  annDeallocPt( queryPt );
                  annDeallocPts( dataPts );
                  delete[] nnIdx;
                  delete[] dists;
                  delete annTree;

                  res.cutIt = false;

                  return res;
                }
    float *curve1 = (float*)malloc( 2*sizeof(float)*sizeContour );
    float *curve2 = (float*)malloc( 2*sizeof(float)*sizeContour );
    int    sizeCurve1, sizeCurve2;
    ipMITKSegmentationSplitContour( contour, sizeContour, res.cutCoords, curve1, sizeCurve1, curve2, sizeCurve2 );
    float clickX = (float)(ofs % line) + 0.5;
    float clickY = (float)(ofs / line) + 0.5;
    if (ipMITKSegmentationIsInsideContour( curve1, sizeCurve1, clickX, clickY )) {
      res.deleteCurve = curve1;
      res.deleteSize = sizeCurve1;
      free( curve2 );
    }
    else {
      res.deleteCurve = curve2;
      res.deleteSize = sizeCurve2;
      free( curve1 );
    }
  }


  free( contour );
  // free ANN stuff:
  annDeallocPt( queryPt );
  annDeallocPts( dataPts );
  delete[] nnIdx;
  delete[] dists;
  delete annTree;

  return res;
}
