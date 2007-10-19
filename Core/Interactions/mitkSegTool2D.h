/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
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

#ifndef mitkSegTool2D_h_Included
#define mitkSegTool2D_h_Included

#include "mitkCommon.h"
#include "mitkSegTool3D.h"

#include "mitkContour.h"
#include "mitkImage.h"
#include "mitkDataTreeNode.h"

#include "mitkStateEvent.h"
#include "mitkPositionEvent.h"

#include "mitkImageCast.h"

namespace mitk
{

class BaseRenderer;

/**
  \brief Abstract base class for segmentation tools.

  \sa Tool
  \sa SegTool3D
  \sa Contour

  \ingroup Interaction
  \ingroup Reliver

  Implements 2D segmentation specific helper methods, that might be of use to
  all kind of 2D segmentation tools. At the moment these are:
   - Determination of the slice where the user paints upon (DetermineAffectedImageSlice)
   - Projection of a 3D contour onto a 2D plane/slice
   - Providing a feedback contour that might be added or removed from the visible scene (SetFeedbackContourVisible).
   - Filling of a contour into a 2D slice

   SegTool2D tries to structure the interaction a bit. If you pass "PressMoveRelease" as the interaction type
   of your derived tool, you might implement the methods OnMousePressed, OnMouseMoved, and OnMouseReleased. 
   Yes, your guess about when they are called is correct.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class SegTool2D : public SegTool3D
{
  public:
    
    mitkClassMacro(SegTool2D, SegTool3D);

  protected:

    SegTool2D(); // purposely hidden
    SegTool2D(const char*); // purposely hidden
    virtual ~SegTool2D();

    virtual bool OnMousePressed (Action*, const StateEvent*);
    virtual bool OnMouseMoved   (Action*, const StateEvent*);
    virtual bool OnMouseReleased(Action*, const StateEvent*);
    virtual bool OnInvertLogic  (Action*, const StateEvent*);

    Contour* GetFeedbackContour();
    void SetFeedbackContour(Contour&);
    void SetFeedbackContourVisible(bool);

    /// Provide values from 0.0 (black) to 1.0 (full color)
    void SetFeedbackContourColor( float r, float g, float b );
    void SetFeedbackContourColorDefault();

    /**
      \brief Calculates for a given Image and PlaneGeometry, which slice of the image (in index corrdinates) is meant by the plane.

      \return false, if no slice direction seems right (e.g. rotated planes)
      \param affectedDimension The image dimension, which is constant for all points in the plane, e.g. Transversal --> 2
      \param affectedSlice The index of the image slice
    */
    bool DetermineAffectedImageSlice( const Image* image, const PlaneGeometry* plane, int& affectedDimension, int& affectedSlice );

    /**
      \brief Extract the slice of an image that the user just scribbles on.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position.
    */
    Image::Pointer GetAffectedImageSliceAs2DImage(const PositionEvent*, const Image* image);

    /**
      \brief Extract the slice of the currently selected working image that the user just scribbles on.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position,
                   or just no working image is selected.
    */
    Image::Pointer GetAffectedWorkingSlice(const PositionEvent*);

    /**
      \brief Extract the slice of the currently selected reference image that the user just scribbles on.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position,
                   or just no reference image is selected.
    */
    Image::Pointer GetAffectedReferenceSlice(const PositionEvent*);

    /**
      \brief Projects a contour onto an image point by point. Converts from world to index coordinates.

      \param correctionForIpSegmentation adds 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    Contour::Pointer ProjectContourTo2DSlice(Image* slice, Contour* contourIn3D, bool correctionForIpSegmentation = false, bool constrainToInside = true);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.
      
      \param correctionForIpSegmentation substracts 0.5 to x and y index coordinates (difference between ipSegmentation and MITK contours)
    */
    Contour::Pointer BackProjectContourFrom2DSlice(Image* slice, Contour* contourIn2D, bool correctionForIpSegmentation = false);


    /**
      \brief Paint a filled contour (e.g. of an ipSegmentation pixel type) into a mitk::Image (or arbitraty pixel type).
      Will not copy the whole filledContourSlice, but only set those pixels in originalSlice to overwritevalue, where the corresponding pixel
      in filledContourSlice is non-zero.
    */
    template<typename TPixel, unsigned int VImageDimension>
    void ItkCopyFilledContourToSlice( itk::Image<TPixel,VImageDimension>* originalSlice, const Image* filledContourSlice, int overwritevalue = 1 );

    /**
      \brief Fill a contour in a 2D slice with a specified pixel value.
    */
    void FillContourInSlice( Contour* projectedContour, Image* sliceImage, int paintingPixelValue = 1 );

    void SliceBasedSegmentationBugMessage( const std::string& message );
 
  private:

    Contour::Pointer      m_FeedbackContour;
    DataTreeNode::Pointer m_FeedbackContourNode;
    bool                  m_FeedbackContourVisible;
    BaseRenderer*         m_LastEventSender;
    unsigned int          m_LastEventSlice;
};

} // namespace

#endif

