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

/*! \mainpage

  \author Tobias Kunert, Thomas Boettger, Tobias Heimann (Deutsches Krebsforschungszentrum, Heidelberg)
  \date October 2002

  The ipMITKSegmentation library manages the results of the
  segmentation process. It provides basic capabilities
  to modify segmentation images. It includes also an
  undo mechanism which allows to trace back the changes. 

*/

#ifndef IPSEGMENTATION_H
#define IPSEGMENTATION_H

#include <ipPic/ipPic.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

  /*!
    \brief Defines the data type of the segmentation images.
    */
#define ipMITKSegmentationTYPE ipUInt1_t
#define ipMITKSegmentationTYPE_ID ipPicUInt
#define ipMITKSegmentationBPE 8
#define tagSEGMENTATION_EMPTY "SEGMENTATION_EMPTY"

  /*!
    \brief Initializes the segmentation.
    @param image the original image which will be segmented
    @return the segmentation.
    */
  extern ipPicDescriptor* ipMITKSegmentationNew (ipPicDescriptor* image);

  /*!
    \brief Destroys the segmentation.
    @param segmentation the segmentation.
    */
  extern void ipMITKSegmentationFree (ipPicDescriptor* segmentation);

  /*!
    \brief Clears the segmentation data, i.e. the pixels will
    be set to zero.
    @param segmentation the segmentation
    */
  extern void ipMITKSegmentationClear (ipPicDescriptor* segmentation);

  /*!
    \brief Interpolates the shape of segmentations.
    @param pic1,pic2 the images of the segmentation
    @param ratio the ratio of the images, the ratios 0.0 and 1.0 will 
    produce pic1 and pic2, accordingly.
    */
  extern ipPicDescriptor* ipMITKSegmentationInterpolate (ipPicDescriptor* pic1, ipPicDescriptor* pic2, const ipFloat4_t ratio);

  /*!
    \brief The type of logical operation.
    */
  enum LogicalOp {
    IPSEGMENTATION_OP    /*! copy of input image */,
    IPSEGMENTATION_AND   /*! logical and operator */,
    IPSEGMENTATION_OR    /*! the or operator */,
    IPSEGMENTATION_XOR   /*! the exclusive-or operator */
  };

  /*!
    \brief Combines the region with the segmentation image.
    @param points the array of points, each point is represented
    by a 2-D vector (x, y)
    @param segmentation the segmentation.
    @param num the number of points
    @param mask the image which restricts the operation to a particular region 
    @param operation the operation which will be performed (see above)
    @param value the operand value of the operation
    \note The last point is automatically connected with the first one.
    */
  extern void ipMITKSegmentationCombineRegion (ipPicDescriptor* segmentation, const ipInt4_t* const points, const int num, ipPicDescriptor* mask, const int operation, int value);

  /*!
    \brief Enables the undo operation for the specified segmentation.
    @param segmentation the segmentation 
    @param level the number of undo levels
    */
  extern void ipMITKSegmentationUndoEnable (ipPicDescriptor* segmentation, const ipUInt1_t level);

  /*!
    \brief Disables the undo operation for the specified segmentation.
    The available data will be discarded.
    @param segmentation the segmentation 
    */
  extern void ipMITKSegmentationUndoDisable (ipPicDescriptor* segmentation);

  /*!
    \brief Checks if the undo operation is enabled.
    @param segmentation the segmentation
    */
  extern ipBool_t ipMITKSegmentationUndoIsEnabled (ipPicDescriptor* segmentation);

  /*!
    \brief Checks if any data for undo is available.
    pending?
    */
  extern ipBool_t ipMITKSegmentationUndoAvailable (ipPicDescriptor* segmentation);

  /*!
    \brief Save the segmentation image before it is changed.
    @param segmentation the segmentation
    */
  extern void ipMITKSegmentationUndoSave (ipPicDescriptor* segmentation);

  /*!
    \brief Steps to the previous undo level. The data which has been saved 
    before the last modifications will be restored.
    */
  extern void ipMITKSegmentationUndo (ipPicDescriptor* segmentation);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

/*!
  Starts a 4 neighbourhood region growing at startOfs (y*picWidth+x) of the 2D image src.
  If relativeBounds is true, the region grows in [base-lowerBound, base+upperBound], in which base is the average
  color of the 9 pixels around startOfs. If relativeBounds is false, the region grows in [lowerBound, upperBound].
  If maxIterations is > 0, the growing process is stopped after maxIterations.
  If segBuffer is 0, new memory for the segmented image is allocated and returned, else the segBuffer is used
  to store the result (has to be an 8-bit datatype, e.g. ipUInt1_t).
  histBuffer must be 0 or a pointer to a 16-bit ipPicUInt image of the same size as src. In case of the latter, 
  history data is written to that buffer: the seed pixel gets a 1, all direct neighbours 2 etc. The buffer is
  not cleared in this function and can thus hold history data of several growing processes in different areas.
  */
extern ipPicDescriptor* ipMITKSegmentationGrowRegion4N( ipPicDescriptor *src, int startOfs, bool relativeBounds, float lowerBound, float upperBound, int maxIterations, ipPicDescriptor *segBuffer, ipPicDescriptor *histBuffer=0 );

/*!
  Same as the other ipMITKSegmentationGrowRegion4N with two additional return values:
  contourOfs holds the lowest point of the resulting region and is thus guaranteed to be part of the contour
  Take care: if the region could not grow at all (e.g. with fixed borders) contourOfs will be -1 !!!
  startCol holds the color that was used as base if relativeBounds is true
  */
extern ipPicDescriptor* ipMITKSegmentationGrowRegion4N( ipPicDescriptor *src, int startOfs, bool relativeBounds, float lowerBound, float upperBound, int maxIterations, ipPicDescriptor *segBuffer, int &contourOfs, float &startCol, ipPicDescriptor *histBuffer=0 );

/*!
  Replaces the 4 neighbourhood region around startOfs (y*picWidth+x) of the 2D segmented image seg with newValue.
  Seg has to be an 8-bit datatype, e.g. ipUInt1_t.
  Returns the number of replaced pixels. If newValue is the same as the old value, the function returns 0.
  */
extern int ipMITKSegmentationReplaceRegion4N( ipPicDescriptor *seg, int startOfs, ipInt1_t newValue );

/*!
  Same as above, but for the 8 neighbourhood contour.
  */
extern float* ipMITKSegmentationGetContour8N( const ipPicDescriptor *seg, int startOfs, int &numPoints, int &sizeBuffer, float *pointBuffer=0 );


typedef struct {
  float *traceline;    // x/y points describing the calculated path - memory has to be freed!
  bool  *onGradient;   // flags for each point if path was calculated based on gradient (true) or on max distance (false) - memory has to be freed!
  int    numPoints;    // number of points in the path
  int    absMin;       // indexes the point at the narrowest part of the path
  bool  cutIt;         // true if a leak has been found
  float cutCoords[4];  // x1,y1,x2,y2 of the two contour points where the leak can be cut
  float *deleteCurve;  // x/y points of the closed contour describing the leak region
  int   deleteSize;    // number of points in this contour
} tCutResult;

/*!
  Calculates a path from ofs to the origin of the growing process, when possible on the skeleton of the
  segmented area. The narrowest part of this path is used to create two cut points that are part of the contour
  and can later be used to split the segmentation. This funtion is useful for detecting and removing leaks in
  region growing.
  */
extern tCutResult ipMITKSegmentationGetCutPoints( ipPicDescriptor *seg, ipPicDescriptor *history, int ofs );

/*!
  Creates a grower history (i.e. the order in which a region grower would have segmented the area) of the given
  segmentation, origin of the grower is startOfs. histBuffer must be a 16bit unsigned int or 0, in the latter
  case the pic is created.
  */
extern ipPicDescriptor* ipMITKSegmentationCreateGrowerHistory( ipPicDescriptor *seg, int startOfs, ipPicDescriptor *histBuffer );

/*!
  Splits a contour in two parts. contour is the original contour, cutCoords a pointer to an x1-y1-x2-y2 array that
  specifies the two points where the contour should be cut (these cutpoints must be points of the original contour!).
  part1 and part2 have to be float arrays of a sufficient size to store the two parts. The sizes of the two parts
  are returned in size1 and size2.
  */
extern void ipMITKSegmentationSplitContour( float *contour, int sizeContour, float *cutCoords, float *part1, int &size1, float *part2, int &size2 );

/*!
  Returns true if the specified point lies insede the contour, else returns false.
  */
extern bool ipMITKSegmentationIsInsideContour( float *contour, int sizeContour, float x, float y );

#endif
