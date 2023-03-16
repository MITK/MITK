/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelLiveWireInteractor_h
#define mitkContourModelLiveWireInteractor_h

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

    virtual void SetWorkingImage(mitk::Image *_arg);

    void ConnectActionsAndFunctions() override;

  protected:
    ContourModelLiveWireInteractor();
    ~ContourModelLiveWireInteractor() override;

    /// \brief Select/ add and select vertex to modify contour and prepare for modification of contour.
    bool OnCheckPointClick(const InteractionEvent *interactionEvent) override;

    /// \brief Update contour when point is moved.
    void OnMovePoint(StateMachineAction *, InteractionEvent *interactionEvent) override;
    /// \brief Add a new control point.
    void OnAddPoint(StateMachineAction*, InteractionEvent* interactionEvent) override;
    /// \brief Delete selected vertex and recompute contour.
    void OnDeletePoint(StateMachineAction *, InteractionEvent *interactionEvent) override;
    /// \brief Finish modification of contour.
    void OnFinishEditing(StateMachineAction *, InteractionEvent *interactionEvent) override;

    /// \brief Split contour into a part before the selected vertex and after the selected vertex
    void SplitContourFromSelectedVertex(mitk::ContourModel *srcContour,
                                        const mitk::ContourModel::VertexType *nextPoint,
                                        const mitk::ContourModel::VertexType *previousPoint,
                                        int timestep);
    /// \brief Set repulsive points which should not be changed during editing of the contour.
    void SetRepulsivePoints(const mitk::ContourModel::VertexType *nextPoint,
                            mitk::ContourModel *contour,
                            int timestep);

    mitk::ImageLiveWireContourModelFilter::Pointer m_LiveWireFilter;
    mitk::Image::Pointer m_WorkingSlice;

    mitk::Point3D m_NextActiveVertexDown;
    mitk::Point3D m_NextActiveVertexUp;

    mitk::ContourModel::VertexIterator m_NextActiveVertexDownIter;
    mitk::ContourModel::VertexIterator m_NextActiveVertexUpIter;

    std::vector<itk::Index<2>> m_ContourBeingModified;

    mitk::ContourModel::Pointer m_ContourLeft;
    mitk::ContourModel::Pointer m_ContourRight;
  };

} // namespace mitk

#endif
