/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTubeGraphDataInteractor3D_h_
#define mitkTubeGraphDataInteractor3D_h_

#include <MitkTubeGraphExports.h>

#include <mitkBaseRenderer.h>
#include <mitkDataInteractor.h>

#include "mitkTubeGraph.h"
#include "mitkTubeGraphProperty.h"

namespace mitk
{
  // Define events for TubeGraph interaction notifications
  itkEventMacro(SelectionChangedTubeGraphEvent, itk::AnyEvent);

  /**
  * \brief
  *
  * \ingroup Interaction
  */
  // Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
  class MITKTUBEGRAPH_EXPORT TubeGraphDataInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(TubeGraphDataInteractor, DataInteractor);
    itkNewMacro(Self);

    /**
    * Describes, which activation modes are available based on the
    * currently picked tube:
    *
    *   \li <b>None</b> means "no tube is active"
    *   \li <b>Single</b> means "only the picked tube is active"
    *   \li <b>ToRoot</b> means "all tubes from the picked on down to the root of the tube graph are active"
    *   \li <b>ToPeriphery</b> means "all tubes included in the subgraph of the currently picked vessel are active"
    *   \li <b>Points</b> means "shortes path between two picked tubes are active"
    *   \li <b>Multiple</b> means "all picked tubes are active"
    */
    enum ActivationMode
    {
      None = 0,
      Single,
      ToRoot,
      ToPeriphery,
      Points,
      Multiple
    };

    enum ActionMode
    {
      AttributationMode = 0,
      AnnotationMode,
      EditMode,
      RootMode,
      InformationMode
    };

    void SetActivationMode(const ActivationMode &activationMode);
    ActivationMode GetActivationMode();

    void SetActionMode(const ActionMode &actionMode);
    ActionMode GetActionMode();

    void ResetPickedTubes();

	mitk::Point3D GetLastPickedPosition();

  protected:
    TubeGraphDataInteractor();
    ~TubeGraphDataInteractor() override;

    /**
    * Here actions strings from the loaded state machine pattern are mapped to functions of
    * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
    */
    void ConnectActionsAndFunctions() override;

    /**
    * This function is called when a DataNode has been set/changed.
    */
    void DataNodeChanged() override;

    /**
    * Initializes the movement, stores starting position.
    */
    virtual bool CheckOverTube(const InteractionEvent *);
    virtual void SelectTube(StateMachineAction *, InteractionEvent *);
    virtual void DeselectTube(StateMachineAction *, InteractionEvent *);

    void SelectTubesByActivationModus();
    void UpdateActivation();

  private:
    std::vector<TubeGraph::TubeDescriptorType> GetTubesToRoot();
    std::vector<TubeGraph::TubeDescriptorType> GetTubesBetweenPoints();
    std::vector<TubeGraph::TubeDescriptorType> GetPathToPeriphery();
    std::vector<TubeGraph::TubeDescriptorType> GetPathBetweenTubes(const TubeGraph::TubeDescriptorType &start,
                                                                   const TubeGraph::TubeDescriptorType &end);

    TubeGraph::Pointer m_TubeGraph;
    TubeGraphProperty::Pointer m_TubeGraphProperty;
    TubeGraph::TubeDescriptorType m_LastPickedTube;
    TubeGraph::TubeDescriptorType m_SecondLastPickedTube;
    ActivationMode m_ActivationMode;
    ActionMode m_ActionMode;
    mitk::TubeElement *m_LastPickedElement = nullptr;
  };
}
#endif
