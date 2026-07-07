/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataInteractor_h
#define mitkDataInteractor_h

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include <mitkEventStateMachine.h>
#include <mitkWeakPointer.h>
#include <string>

namespace mitk
{
  class DataNode;

  itkEventMacroDeclaration(DataInteractorEvent, itk::AnyEvent);

  /** Triggered when interaction is started */
  itkEventMacroDeclaration(StartInteraction, DataInteractorEvent);

  /** Triggered when result is stored in mitk::DataNode */
  itkEventMacroDeclaration(ResultReady, DataInteractorEvent);

    enum ProcessEventMode {
      REGULAR = 0,
      GRABINPUT = 1,
      PREFERINPUT = 2,
      CONNECTEDMOUSEACTION = 3
    };

  /**
   * \brief Base class for interactors that operate on DataNodes.
   *
   * DataInteractor provides the interface required for an interactor to work
   * together with the Dispatcher. Each DataInteractor is associated with exactly
   * one DataNode and processes events according to its loaded state machine pattern.
   *
   * To implement a new interactor, subclass DataInteractor and override
   * ConnectActionsAndFunctions() to bind state machine action names to member functions.
   *
   * The processing priority is determined by the "layer" property of the
   * associated DataNode (higher layers are handled first).
   *
   * \sa EventStateMachine
   * \sa Dispatcher
   * \sa DataNode
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT DataInteractor : public EventStateMachine
  {
  public:
    /** \brief Internal signal name to request deactivation of this interactor. */
    static const std::string IntDeactivateMe;
    /** \brief Internal signal name indicating the pointer left the widget area. */
    static const std::string IntLeaveWidget;
    /** \brief Internal signal name indicating the pointer entered the widget area. */
    static const std::string IntEnterWidget;

    mitkClassMacro(DataInteractor, EventStateMachine);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /**
     * \brief Get the DataNode associated with this interactor.
     * \return Pointer to the DataNode, or nullptr if none is set.
     */
    DataNode *GetDataNode() const;

    /**
     * \brief Set the DataNode that this interactor operates on.
     *
     * Removes the interactor from any previously associated DataNode,
     * assigns this interactor to the new DataNode, and calls DataNodeChanged().
     *
     * \param[in] dataNode The DataNode to associate. Can be nullptr to detach.
     */
    virtual void SetDataNode(DataNode *dataNode);

    /**
     * \brief Get the rendering layer of the associated DataNode.
     *
     * The layer determines the priority in event dispatching (higher layers first).
     *
     * \return The integer layer value, or -1 if no DataNode is set.
     */
    int GetLayer() const;

    /**
     * \brief Get the current event processing mode from the state machine's current state.
     *
     * The mode determines how the Dispatcher handles event distribution:
     * - REGULAR: normal dispatching
     * - PREFERINPUT: this interactor is preferred but others can still receive events
     * - GRABINPUT: only this interactor receives events
     *
     * \return The current ProcessEventMode.
     */
    ProcessEventMode GetMode() const;

  protected:
    DataInteractor();
    ~DataInteractor() override;

    /**
     * @brief Overwrite this function to connect actions from StateMachine description with functions.
     *
     * Following example shows how to connect the 'addpoint' action from the StateMachine XML description using the
     CONNECT_FUNCTION macro
     * with the AddPoint() function in the TestInteractor.
     * @code
     * void mitk::TestInteractor::ConnectActionsAndFunctions()
     {
     CONNECT_FUNCTION("addpoint", AddPoint);
     }
     * @endcode
     */
    void ConnectActionsAndFunctions() override;

    /** \brief Is called when a DataNode is initially set or changed
     *  To be implemented by sub-classes for initialization code which require a DataNode.
     *  \note New DataInteractors usually are expected to have the focus, but this only works if they have the highest
     * Layer,
     *  since empty DataNodes have a layer of -1, the DataNode must be filled here in order to get a layer assigned.
     *  \note Is also called when the DataNode is set to nullptr.
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
