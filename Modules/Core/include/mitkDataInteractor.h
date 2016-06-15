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

#ifndef MITKDATAINTERACTOR_H_
#define MITKDATAINTERACTOR_H_

#include <mitkCommon.h>
#include <mitkEventStateMachine.h>
#include <mitkWeakPointer.h>
#include <MitkCoreExports.h>
#include <string>

namespace mitk
{
  class DataNode;

  itkEventMacro(DataInteractorEvent, itk::AnyEvent)

  /** Triggered when interaction is started */
  itkEventMacro(StartInteraction, DataInteractorEvent)

  /** Triggered when result is stored in mitk::DataNode */
  itkEventMacro(ResultReady, DataInteractorEvent)

  enum ProcessEventMode
  {
    REGULAR = 0,
    GRABINPUT = 1,
    PREFERINPUT = 2,
    CONNECTEDMOUSEACTION = 3
  };

  /**
  * \brief Base class from with interactors that handle DataNodes are to be derived.
  *
  * Base class from with interactors that handle DataNodes are to be derived.
  * Provides an interface that is relevant for the interactor to work together with the dispatcher.
  * To implement a new interactor overwrite the ConnectActionsAndFunctions to connect the actions.
  */
  class MITKCORE_EXPORT DataInteractor : public EventStateMachine
  {
  public:
    // Predefined internal events/signals
    static const std::string IntDeactivateMe;
    static const std::string IntLeaveWidget;
    static const std::string IntEnterWidget;

    mitkClassMacro(DataInteractor, EventStateMachine)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    DataNode* GetDataNode() const;
    virtual void SetDataNode(DataNode* dataNode); // TODO: Remove virtual, use DataNodeChanged instead in subclasses

    int GetLayer() const;

    ProcessEventMode GetMode() const;

  protected:
    DataInteractor();
    virtual ~DataInteractor();

    /**
     * @brief Overwrite this function to connect actions from StateMachine description with functions.
     *
     * Following example shows how to connect the 'addpoint' action from the StateMachine XML description using the CONNECT_FUNCTION macro
     * with the AddPoint() function in the TestInteractor.
     * @code
     * void mitk::TestInteractor::ConnectActionsAndFunctions()
     {
     CONNECT_FUNCTION("addpoint", AddPoint);
     }
     * @endcode
     */
    virtual void ConnectActionsAndFunctions() override;

    /** \brief Is called when a DataNode is initially set or changed
     *  To be implemented by sub-classes for initialization code which require a DataNode.
     *  \note New DataInteractors usually are expected to have the focus, but this only works if they have the highest Layer,
     *  since empty DataNodes have a layer of -1, the DataNode must be filled here in order to get a layer assigned.
     *  \note Is also called when the DataNode is set to NULL.
     */
    virtual void DataNodeChanged();

    /**
     * @brief Sends StartInteraction event via the mitk::DataNode
     */
    void virtual NotifyStart();

    /**
     * @brief NotifyResultReady Sends ResultReady event via the mitk::DataNode
     *
     * Use to get notfied when the mitk::DataNode is in a ready state for further processing.
     */
    void virtual NotifyResultReady();

  private:
    WeakPointer<DataNode> m_DataNode;
  };
}
#endif
