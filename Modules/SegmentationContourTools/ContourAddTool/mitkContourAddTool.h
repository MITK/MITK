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

#ifndef mitkContourAddTool_h_Included
#define mitkContourAddTool_h_Included

#include "mitkCommon.h"
#include "SegmentationContourToolsExports.h"
#include "mitkSegTool2D.h"
#include "mitkContourModel.h"
//#include <mitkContourModelLiveWireInteractor.h>
//#include <mitkImageLiveWireContourModelFilter.h>

class ModuleResource;

namespace mitk
{

class SegmentationContourTools_EXPORT ContourAddTool
{
  public:

//    mitkClassMacro(ContourAddTool, SegTool2D);
//    itkNewMacro(ContourAddTool);

/*
    virtual const char** GetXPM() const;
    virtual ModuleResource GetCursorIconResource() const;
    ModuleResource GetIconResource() const;

    virtual const char* GetName() const;

    /// \brief Convert all current contour objects to binary segmentation image.
    void ConfirmSegmentation();

  protected:

    LiveWireTool2D();
    virtual ~LiveWireTool2D();
*/
    /**
    * \brief Calculates how good the data, this statemachine handles, is hit by the event.
    *
    */
    /*
    virtual float CanHandleEvent( StateEvent const *stateEvent) const;

    virtual void Activated();
    virtual void Deactivated();

    /// \brief Memory release from all used contours
    virtual void ClearContours();

    /// \brief Initialize tool
    virtual bool OnInitLiveWire (Action*, const StateEvent*);

    /// \brief Add a control point and finish current segment
    virtual bool OnAddPoint   (Action*, const StateEvent*);

    /// \brief Actual LiveWire computation
    virtual bool OnMouseMoved(Action*, const StateEvent*);

    /// \brief Check double click on first control point to finish the LiveWire tool
    virtual bool OnCheckPoint(Action*, const StateEvent*);

    /// \brief Finish LiveWire tool
    virtual bool OnFinish(Action*, const StateEvent*);

    /// \brief Close the contour
    virtual bool OnLastSegmentDelete(Action*, const StateEvent*);

    /// \brief Don't use dynamic cost map for LiveWire calculation
    virtual bool OnMouseMoveNoDynamicCosts(Action*, const StateEvent*);

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

    // the contour for the editing portion
    mitk::ContourModel::Pointer m_EditingContour;

    //the corresponding datanode
    mitk::DataNode::Pointer m_EditingContourNode;

    // the corresponding contour interactor
    mitk::ContourModelLiveWireInteractor::Pointer m_ContourInteractor;

    //the current reference image
    mitk::Image::Pointer m_WorkingSlice;

    // the filter for live wire calculation
    mitk::ImageLiveWireContourModelFilter::Pointer m_LiveWireFilter;

    bool m_CreateAndUseDynamicCosts;

    std::vector< std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> > m_WorkingContours;
    std::vector< std::pair<mitk::DataNode::Pointer, mitk::PlaneGeometry::Pointer> > m_EditingContours;
    std::vector< mitk::ContourModelLiveWireInteractor::Pointer > m_LiveWireInteractors;

    template<typename TPixel, unsigned int VImageDimension>
    void FindHighestGradientMagnitudeByITK(itk::Image<TPixel, VImageDimension>* inputImage, itk::Index<3> &index, itk::Index<3> &returnIndex);
*/
};

} // namespace

#endif
