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

#ifndef _mitk_TubeGraphInteractor_h
#define _mitk_TubeGraphInteractor_h

#include "MitkTubeGraphExports.h"

#include "mitkInteractor.h"

#include "mitkDataNode.h"
#include "mitkTubeGraph.h"
#include "mitkTubeGraphProperty.h"

namespace mitk
{
  //#pragma GCC visibility push(default)

  // Define events for TubeGraph interaction notifications
  itkEventMacro( SelectionChangedTubeGraphEvent, itk::AnyEvent );

  //#pragma GCC visibility pop

  class MitkTubeGraph_EXPORT TubeGraphInteractor : public Interactor
  {

  public:

    mitkClassMacro( TubeGraphInteractor, Interactor );
    mitkNewMacro2Param(Self, const char*, DataNode*);

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
    *
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

    /**
    * Calculates, if the given event applies to the current tube graph. This is
    * done via checking if m_DataNode == CurrentlyActiveDataNode ) If yes,
    * the interactor returns 1.0f, or 0.0f otherwise.
    */
    virtual float CanHandleEvent( StateEvent const* stateEvent ) const;

    /**
    * @param mitk::DataNode set the member variable m_CurrentSelectedDataNode.
    */
    static void SetCurrentSelectedDataNode( DataNode* node );

    void SetActivationMode(const ActivationMode& activationMode);
    ActivationMode GetActivationMode();

    void SetActionMode(const ActionMode& actionMode);
    ActionMode GetActionMode();

    void ResetPickedTubes();

    // /**
    // * @param bool set the member varible m_PermissionBarrier.
    // */
    //void SetPermissionBarrier(bool);

  protected:
    /**
    * Constructor.
    * @param type State machine pattern from StateMachine.xml
    * @param dataNode Belonging DataNode
    */
    TubeGraphInteractor( const char * type, DataNode* dataNode );
    virtual ~TubeGraphInteractor();

    virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

    void SelectTubesByActivationModus();
    void UpdateActivation();

    std::vector<TubeGraph::TubeDescriptorType> GetTubesToRoot();
    std::vector<TubeGraph::TubeDescriptorType> GetTubesBetweenPoints();
    std::vector<TubeGraph::TubeDescriptorType> GetPathToPeriphery();
    std::vector<TubeGraph::TubeDescriptorType> GetPathBetweenTubes(const TubeGraph::TubeDescriptorType& start,const TubeGraph::TubeDescriptorType& end);


    /**
    * Shared by all instances of TubeGraphInteractor
    */
    static DataNode*               m_CurrentSelectedDataNode;
    TubeGraph::Pointer             m_TubeGraph;
    TubeGraphProperty::Pointer     m_TubeGraphProperty;
    /*bool m_PermissionBarrier;*/
    TubeGraph::TubeDescriptorType  m_LastPickedTube;
    TubeGraph::TubeDescriptorType  m_SecondLastPickedTube;
    ActivationMode                 m_ActivationMode;
    ActionMode                     m_ActionMode;
  };


}//namespace

#endif
