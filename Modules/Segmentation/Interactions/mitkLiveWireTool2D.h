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

#ifndef mitkCorrectorTool2D_h_Included
#define mitkCorrectorTool2D_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkSegTool2D.h"
#include <mitkContourModel.h>
#include <mitkContourModelLiveWireInteractor.h>
#include <mitkImageLiveWireContourModelFilter.h>

namespace us {
class ModuleResource;
}

namespace mitk
{


/**
  \brief A 2D segmentation tool based on LiveWire approach.
  The contour between the last user added point and the current mouse position
  is computed by searching the shortest path according to specific features of
  the image. The contour thus snappest to the boundary of objects.


  \sa SegTool2D
  \sa ImageLiveWireContourModelFilter

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  \warning Only to be instantiated by mitk::ToolManager.
*/
class Segmentation_EXPORT LiveWireTool2D : public SegTool2D
{
  public:

    mitkClassMacro(LiveWireTool2D, SegTool2D);
    itkNewMacro(LiveWireTool2D);

    virtual const char** GetXPM() const;
    virtual us::ModuleResource GetCursorIconResource() const;
    us::ModuleResource GetIconResource() const;

    virtual const char* GetName() const;

  protected:

    LiveWireTool2D();
    virtual ~LiveWireTool2D();

    void ConnectActionsAndFunctions();

    virtual void Activated();
    virtual void Deactivated();

    /// \brief Initialize tool
    virtual bool OnInitLiveWire ( StateMachineAction*, InteractionEvent* interactionEvent );

    /// \brief Add a control point and finish current segment
    virtual bool OnAddPoint   ( StateMachineAction*, InteractionEvent* interactionEvent );

    /// \brief Actual LiveWire computation
    virtual bool OnMouseMoved( StateMachineAction*, InteractionEvent* interactionEvent );

    /// \brief Check double click on first control point to finish the LiveWire tool
    virtual bool OnCheckPoint( const InteractionEvent* interactionEvent );

    /// \brief Finish LiveWire tool
    virtual bool OnFinish( StateMachineAction*, InteractionEvent* interactionEvent );

    /// \brief Close the contour
    virtual bool OnLastSegmentDelete( StateMachineAction*, InteractionEvent* interactionEvent );

    /// \brief Don't use dynamic cost map for LiveWire calculation
    virtual bool OnMouseMoveNoDynamicCosts( StateMachineAction*, InteractionEvent* interactionEvent );

    /// \brief Finish contour interaction.
    void FinishTool();


    //the contour already set by the user
    mitk::ContourModel::Pointer m_Contour;
    //the corresponding datanode
    mitk::DataNode::Pointer m_ContourModelNode;

    //the current LiveWire computed contour
    mitk::ContourModel::Pointer m_LiveWireContour;
    //the corresponding datanode
    mitk::DataNode::Pointer m_LiveWireContourNode;

    mitk::ContourModel::Pointer m_EditingContour;
    mitk::DataNode::Pointer m_EditingContourNode;

    //the current reference image
    mitk::Image::Pointer m_WorkingSlice;

    mitk::ImageLiveWireContourModelFilter::Pointer m_LiveWireFilter;

    bool m_CreateAndUseDynamicCosts;

    int m_TimeStep;

    std::vector< std::pair<mitk::DataNode*, mitk::PlaneGeometry::Pointer> > m_Contours;

    template<typename TPixel, unsigned int VImageDimension>
    void FindHighestGradientMagnitudeByITK(itk::Image<TPixel, VImageDimension>* inputImage, itk::Index<3> &index, itk::Index<3> &returnIndex);
};

} // namespace

#endif
