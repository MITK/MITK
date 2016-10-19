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

  class MITKSIMULATION_EXPORT SimulationInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(SimulationInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

  protected:
    void ConnectActionsAndFunctions() override;
    void DataNodeChanged() override;

  private:
    SimulationInteractor();
    ~SimulationInteractor();

    void StartInteraction(const std::string& type, InteractionPositionEvent* event);
    void StartPrimaryInteraction(StateMachineAction* action, InteractionEvent* event);
    void StartSecondaryInteraction(StateMachineAction* action, InteractionEvent* event);
    void ExecuteInteraction(StateMachineAction* action, InteractionEvent* event);
    void StopInteraction(StateMachineAction* action, InteractionEvent* event);
    bool IsInteractionPerformerNotNull(const InteractionEvent* event);

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
