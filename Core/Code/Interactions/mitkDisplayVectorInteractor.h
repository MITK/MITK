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


#ifndef mitkDisplayVectorInteractor_h
#define mitkDisplayVectorInteractor_h

#include <MitkExports.h>
#include "mitkEventObserver.h"

namespace mitk {
  /**
  *@brief Interactor for displaying different slices in orthogonal views.
  * This includes the interaction of Zooming and Panning.
  * @ingroup Interaction
  **/
  class MITK_CORE_EXPORT DisplayVectorInteractor : public EventObserver
  {
  public:
    mitkClassMacro(DisplayVectorInteractor, StateMachine);
    itkNewMacro(Self);

    virtual void Notify(InteractionEvent::Pointer interactionEvent,bool isHandled);
  protected:
    DisplayVectorInteractor();
    virtual ~DisplayVectorInteractor();

    void ConnectActionsAndFunctions();
    virtual bool Init(StateMachineAction*, InteractionEvent*);
    virtual bool Move(StateMachineAction*, InteractionEvent*);
    virtual bool Zoom(StateMachineAction*, InteractionEvent*);

  private:
    mitk::Point2D m_StartDisplayCoordinate;
    mitk::Point2D m_LastDisplayCoordinate;
    mitk::Point2D m_CurrentDisplayCoordinate;
    mitk::Point2D m_StartCoordinateInMM;
  };
} // namespace mitk
#endif
