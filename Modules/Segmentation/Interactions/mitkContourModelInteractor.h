/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelInteractor_h
#define mitkContourModelInteractor_h

#include "mitkCommon.h"
#include "mitkDataInteractor.h"
#include <MitkSegmentationExports.h>
#include <mitkContourModel.h>
#include <mitkDataNode.h>

namespace mitk
{
  /**
    \brief

    \sa Interactor

    \ingroup Interaction
    \ingroup ToolManagerEtAl


  */
  class MITKSEGMENTATION_EXPORT ContourModelInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(ContourModelInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * Here actions strings from the loaded state machine pattern are mapped to functions of
       * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
       */
      void ConnectActionsAndFunctions() override;

      void SetRestrictedArea(mitk::ContourModel* restrictedArea);

  protected:
    ContourModelInteractor();
    ~ContourModelInteractor() override;

    virtual bool OnCheckPointClick(const InteractionEvent *interactionEvent);
    virtual bool IsHovering(const InteractionEvent *interactionEvent);

    virtual void OnAddPoint(StateMachineAction*, InteractionEvent* interactionEvent);
    virtual void OnDeletePoint(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void OnMovePoint(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void OnMoveContour(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void OnFinishEditing(StateMachineAction *, InteractionEvent *interactionEvent);

    const float eps = 3.0;
    mitk::Point3D m_lastMousePosition;
    mitk::ContourModel::Pointer m_RestrictedArea;
  };

} // namespace
#endif
