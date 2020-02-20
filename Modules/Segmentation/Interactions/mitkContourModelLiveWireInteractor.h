/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelLiveWireInteractor_h_Included
#define mitkContourModelLiveWireInteractor_h_Included

#include "mitkCommon.h"
#include "mitkContourModelInteractor.h"
#include <MitkSegmentationExports.h>

#include <mitkImageLiveWireContourModelFilter.h>

namespace mitk
{
  /**
  \brief

  \sa Interactor
  \sa ContourModelInteractor

  \ingroup Interaction


  \warning Make sure the working image is properly set, otherwise the algorithm for computing livewire contour segments
  will not work!

  */
  class MITKSEGMENTATION_EXPORT ContourModelLiveWireInteractor : public ContourModelInteractor
  {
  public:
    mitkClassMacro(ContourModelLiveWireInteractor, ContourModelInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

        virtual void SetEditingContourModelNode(mitk::DataNode *_arg);

    virtual void SetWorkingImage(mitk::Image *_arg);

    void ConnectActionsAndFunctions() override;

  protected:
    ContourModelLiveWireInteractor();
    ~ContourModelLiveWireInteractor() override;

    bool OnCheckPointClick(const InteractionEvent *interactionEvent) override;
    bool IsHovering(const InteractionEvent *interactionEvent) override;

    void OnMovePoint(StateMachineAction *, InteractionEvent *interactionEvent) override;
    void OnDeletePoint(StateMachineAction *, InteractionEvent *interactionEvent) override;
    void OnFinishEditing(StateMachineAction *, InteractionEvent *interactionEvent) override;

    int SplitContourFromSelectedVertex(mitk::ContourModel *srcContour,
                                       mitk::ContourModel *destContour,
                                       bool fromSelectedUpwards,
                                       int timestep);

    mitk::ImageLiveWireContourModelFilter::Pointer m_LiveWireFilter;
    mitk::Image::Pointer m_WorkingSlice;

    mitk::Point3D m_NextActiveVertexDown;
    mitk::Point3D m_NextActiveVertexUp;

    mitk::ContourModel::VertexIterator m_NextActiveVertexDownIter;
    mitk::ContourModel::VertexIterator m_NextActiveVertexUpIter;

    std::vector<itk::Index<2>> m_ContourBeingModified;

    mitk::DataNode::Pointer m_EditingContourNode;
    mitk::ContourModel::Pointer m_ContourLeft;
    mitk::ContourModel::Pointer m_ContourRight;
  };

} // namespace mitk

#endif // mitkContourModelLiveWireInteractor_h_Included
