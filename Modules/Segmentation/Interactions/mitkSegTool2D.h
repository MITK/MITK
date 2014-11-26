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

#ifndef mitkSegTool2D_h_Included
#define mitkSegTool2D_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkTool.h"
#include "mitkImage.h"

#include "mitkStateEvent.h"
#include "mitkInteractionPositionEvent.h"

#include "mitkPlanePositionManager.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkInteractionConst.h"

#include <mitkDiffSliceOperation.h>


namespace mitk
{

class BaseRenderer;

/**
  \brief Abstract base class for segmentation tools.

  \sa Tool

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Implements 2D segmentation specific helper methods, that might be of use to
  all kind of 2D segmentation tools. At the moment these are:
   - Determination of the slice where the user paints upon (DetermineAffectedImageSlice)
   - Projection of a 3D contour onto a 2D plane/slice

   SegTool2D tries to structure the interaction a bit. If you pass "PressMoveRelease" as the interaction type
   of your derived tool, you might implement the methods OnMousePressed, OnMouseMoved, and OnMouseReleased.
   Yes, your guess about when they are called is correct.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class MitkSegmentation_EXPORT SegTool2D : public Tool
{
  public:

    mitkClassMacro(SegTool2D, Tool);

    /**
      \brief Calculates for a given Image and PlaneGeometry, which slice of the image (in index corrdinates) is meant by the plane.

      \return false, if no slice direction seems right (e.g. rotated planes)
      \param affectedDimension The image dimension, which is constant for all points in the plane, e.g. Axial --> 2
      \param affectedSlice The index of the image slice
    */
    static bool DetermineAffectedImageSlice( const Image* image, const PlaneGeometry* plane, int& affectedDimension, int& affectedSlice );

    /**
     * @brief Updates the surface interpolation by extracting the contour form the given slice.
     * @param slice the slice from which the contour should be extracted
     * @param workingImage the segmentation image
     * @param plane the plane in which the slice lies
     * @param detectIntersection if true the slice is eroded before contour extraction. If the slice is empty after the erosion it is most
     *        likely an intersecting contour an will not be added to the SurfaceInterpolationController
     */
    static void UpdateSurfaceInterpolation (const Image* slice, const Image* workingImage, const PlaneGeometry *plane, bool detectIntersection);

    void SetShowMarkerNodes(bool);

    /**
     * \brief Enables or disables the 3D interpolation after writing back the 2D segmentation result, and defaults to true.
     */
    void SetEnable3DInterpolation(bool);

  protected:

    SegTool2D(); // purposely hidden
    SegTool2D(const char*); // purposely hidden
    virtual ~SegTool2D();

    struct SliceInformation
    {
      mitk::Image::Pointer slice;
      mitk::PlaneGeometry* plane;
      unsigned int timestep;

      SliceInformation () {}

      SliceInformation (mitk::Image* slice, mitk::PlaneGeometry* plane, unsigned int timestep)
      {
        this->slice = slice;
        this->plane = plane;
        this->timestep = timestep;
      }

    };

    /**
    * \brief Filters events that cannot be handle by 2D segmentation tools
    *
    * Current an event is discarded if it was not sent by a 2D renderwindow and if it is
    * not of type InteractionPositionEvent
    */
    virtual bool FilterEvents(InteractionEvent *interactionEvent, DataNode *dataNode);

    /**
      \brief Extract the slice of an image that the user just scribbles on.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position.
    */
    Image::Pointer GetAffectedImageSliceAs2DImage(const InteractionPositionEvent*, const Image* image);

    /**
      \brief Extract the slice of an image cut by given plane.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position.
    */
    Image::Pointer GetAffectedImageSliceAs2DImage(const PlaneGeometry* planeGeometry, const Image* image, unsigned int timeStep);

    /**
      \brief Extract the slice of the currently selected working image that the user just scribbles on.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position,
                   or just no working image is selected.
    */
    Image::Pointer GetAffectedWorkingSlice(const InteractionPositionEvent*);

    /**
      \brief Extract the slice of the currently selected reference image that the user just scribbles on.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position,
                   or just no reference image is selected.
    */
    Image::Pointer GetAffectedReferenceSlice(const InteractionPositionEvent*);

    void WriteBackSegmentationResult (const InteractionPositionEvent*, Image*);

    void WriteBackSegmentationResult (const PlaneGeometry* planeGeometry, Image*, unsigned int timeStep);

    void WriteBackSegmentationResult (std::vector<SliceInformation> sliceList, bool writeSliceToVolume = true);

    void WriteSliceToVolume (SliceInformation sliceInfo);
    /**
      \brief Adds a new node called Contourmarker to the datastorage which holds a mitk::PlanarFigure.
             By selecting this node the slicestack will be reoriented according to the PlanarFigure's Geometry
    */
    int AddContourmarker();

    void InteractiveSegmentationBugMessage( const std::string& message );

    BaseRenderer*         m_LastEventSender;
    unsigned int          m_LastEventSlice;

  private:


    //The prefix of the contourmarkername. Suffix is a consecutive number
    const std::string     m_Contourmarkername;

    bool m_ShowMarkerNodes;
    static bool m_SurfaceInterpolationEnabled;

    DiffSliceOperation* m_doOperation;
    DiffSliceOperation* m_undoOperation;
};

} // namespace

#endif
