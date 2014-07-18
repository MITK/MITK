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

#ifndef mitkSimulationInteractor_h
#define mitkSimulationInteractor_h

#include <mitkDataInteractor.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class InteractionPositionEvent;

  class MitkSimulation_EXPORT SimulationInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(SimulationInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

  protected:
    void ConnectActionsAndFunctions();
    void DataNodeChanged();

  private:
    SimulationInteractor();
    ~SimulationInteractor();

    void StartInteraction(const std::string& type, InteractionPositionEvent* event);
    bool StartPrimaryInteraction(StateMachineAction* action, InteractionEvent* event);
    bool StartSecondaryInteraction(StateMachineAction* action, InteractionEvent* event);
    bool ExecuteInteraction(StateMachineAction* action, InteractionEvent* event);
    bool StopInteraction(StateMachineAction* action, InteractionEvent* event);
    bool IsInteractionPerformerNotNull(const InteractionEvent* event);

    class Impl;
    std::auto_ptr<Impl> m_Impl;
  };
}

#endif
