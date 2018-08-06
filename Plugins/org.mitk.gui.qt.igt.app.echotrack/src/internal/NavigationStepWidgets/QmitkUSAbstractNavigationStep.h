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

#ifndef QMITKUSABSTRACTNAVIGATIONSTEP_H
#define QMITKUSABSTRACTNAVIGATIONSTEP_H

#include <QWidget>

#include "mitkDataStorage.h"
#include "mitkAbstractUltrasoundTrackerDevice.h"

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {
class NavigationDataToNavigationDataFilter;
}

/**
 * \brief Abstract base class for navigation step widgets.
 *
 * This class defines a life cycle for navigation steps. Steps can be activated
 * and deactivated. The first time a step is activated, it is started before
 * the activation. Steps can be stopped, finished and restarted, too.
 *
 * On every state change, the corresponding virtual On... method is called
 * (OnStartStep(), OnActivateStep(), OnFinishStep(), ...). These methods are
 * to implement by concrete navigation step widgets. While a step is in the
 * "active" state, its OnUpdate() method is called periodically.
 */
class QmitkUSAbstractNavigationStep : public QWidget
{
  Q_OBJECT

signals:
  /**
   * \brief Signals that all necessary actions where done.
   * The user can proceed with the next stept after this was signaled.
   */
  void SignalReadyForNextStep();

  /**
   * \brief Signals that it is no longer possible to proceed with following steps.
   * This signal is emmited when the result data of the step was removed or
   * changed, so that the old results are invalid for following steps.
   */
  void SignalNoLongerReadyForNextStep();

  /**
   * \brief Signals that the combined modality was changed by this step.
   * This signal is mainly for steps which creates the combined modality. The
   * new combined modality is given as a parameter.
   */
  void SignalCombinedModalityChanged(itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice>);

  /**
   * \brief Signals that an intermediate result was produced.
   * The properties of the given data node must contain the results. This signal
   * can be used to log the intermediate results of an experiment, for example
   * by using the mitk::USNavigationExperimentLogging.
   */
  void SignalIntermediateResult(const itk::SmartPointer<mitk::DataNode>);

  /**
   * \brief Signals that the settings node was changed.
   * This signal must not be emited in an OnSettingsChanged() method.
   */
  void SignalSettingsNodeChanged(itk::SmartPointer<mitk::DataNode>);

public:
  typedef std::vector< itk::SmartPointer<mitk::NavigationDataToNavigationDataFilter> > FilterVector;

  enum NavigationStepState { State_Stopped, State_Started, State_Active };

  static const char* DATANAME_SETTINGS;
  static const char* DATANAME_IMAGESTREAM;
  static const char* DATANAME_BASENODE;

  explicit QmitkUSAbstractNavigationStep(QWidget *parent = 0);
  ~QmitkUSAbstractNavigationStep();

  /**
   * \brief Getter for the title of the navigation step.
   * This title should be human readable and can be used to display the
   * available steps and the currently active step to the user.
   * The method has to be implemented by a concrete subclass.
   */
  virtual QString GetTitle() = 0;

  /**
   * \brief Indicates if it makes sense to be able to restart the step.
   * This method must be implemented by concrete subclasses if it should not be
   * possible to restart them.
   *
   * \return true for the default implementation
   */
  virtual bool GetIsRestartable();

  /**
   * \brief Called every time the settings for the navigation process where changed.
   * This method may be implemented by a concrete subclass. The default
   * implementation does nothing.
   */
  virtual void OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> /*settingsNode*/) { }

  /**
   * \brief Getter for navigation data filters of the navigation step.
   * This method may be implemented by a concrete subclass. The default
   * implementation returns an empty vector.
   *
   * \return all navigation data filters that should be updated during the navigation process
   */
  virtual FilterVector GetFilter();

  /**
   * \brief Sets the data storage for the exchange of results between navigation steps.
   */
  void SetDataStorage(itk::SmartPointer<mitk::DataStorage> dataStorage);

  /**
   * \brief Sets the combined modality for the navigation step.
   * OnSetCombinedModality() is called internal.
   */
  void SetCombinedModality(itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice> combinedModality);

  /**
   * \brief Should be called to start the navigation step.
   * \return true if the state was 'stopped' before and OnStartStep() returns true
   */
  bool StartStep();

  /**
   * \brief Should be called to stop the navigation step.
   * \return true if the state was 'started' (or 'active') and OnStopStep() (and OnDeactivateStep()) return true
   */
  bool StopStep();

  /**
   * \brief Should be called to restart the navigation step.
   * \return true if OnRestartStep() returns true
   */
  bool RestartStep();

  /**
   * \brief Should be called to finish the navigation step.
   * The state has to be 'active' before and is 'started' afterwards.
   *
   * \return true if the state was 'active' and DeactivateStep() and OnFinishStep() return true
   */
  bool FinishStep();

  /**
   * \brief Should be called to activate the navigation step.
   * The step gets started before if it was stopped.
   *
   * \return true if the state wasn't 'activated' and OnActivateStep() returns true
   */
  bool ActivateStep();

  /**
   * \brief Should be called to deactivate the navigation step.
   * \return true if the state was 'activated' and OnDeactivateStep() returns true
   */
  bool DeactivateStep();

  /**
   * \brief Should be called periodically while the navigation step is active.
   * Internal, the method OnUpdate() is called.
   */
  void Update();

  /**
   * \brief Get the current state of the navigation step.
   * \return State_Stopped, State_Started or State_Active
   */
  NavigationStepState GetNavigationStepState();

protected:
  /**
   * \brief Called when the navigation step gets started.
   * This method has to be implemented by a concrete subclass to handle actions
   * necessary for starting the step (e.g. initializing that has only to be done
   * once).
   *
   * \return if the actions were done successfully
   */
  virtual bool OnStartStep() = 0;

  /**
   * \brief Callen when the navigation step gets stopped.
   * This method may be implemented by a concrete subclass to handle actions
   * necessary for stopping the step (e.g. cleanup). The default implementation
   * does nothing.
   *
   * \return if the actions were done successfully; true on every call for the default implementation
   */
  virtual bool OnStopStep() { return true; }

  /**
   * \brief Called when restarting a navigation step.
   * This method may be implemented by a concrete subclass to handle actions
   * necessary for restarting the navigation step. The default implementations
   * calls OnStopStep() followed by OnStartStep().
   *
   * \return if the actions were done successfully
   */
  virtual bool OnRestartStep();

  /**
   * \brief Called when all necessary actions for the step where done.
   * This method has to be implemented by a concrete subclass to handle actions
   * necessary for finishing the navigation step.
   *
   * \return if the actions were done successfully
   */
  virtual bool OnFinishStep() = 0;

  /**
   * \brief Called when the navigation step gets activated.
   * This method has to be implemented by a concrete subclass to handle actions
   * necessary on activating the navigation step.
   *
   * \return if the actions were done successfully
   */
  virtual bool OnActivateStep() = 0;

  /**
   * \brief Called when the navigation step gets deactivated (-> state started).
   * This method may be implemented by a concrete subclass to handle actions
   * necessary on deactivating the navigation step, which means switching to
   * another step.
   *
   * \return if the actions were done successfully; true on every call for the default implementation
   */
  virtual bool OnDeactivateStep() { return true; }

  /**
   * \brief Called periodically while a navigation step is active.
   * This method has to be implemented by a concrete subclass to handle all
   * periodic actions during the navigation step.
   */
  virtual void OnUpdate() = 0;

  /**
   * \brief Called every time SetCombinedModality() was called.
   * This method may be implemented by a concrete subclass to handle this event.
   * The default implementation does nothing.
   */
  virtual void OnSetCombinedModality() { }

  /**
   * \brief Returns the data storage set for the navigation step.
   *
   * \param throwNull if this method should throw an exception when the data storage is null (default: true)
   * \return the data storage set by SetDataStorage(); can only be null if the parameter throwNull is set to false
   * \throws mitk::Exception if the data storage is null and the parameter throwNull is set to true
   */
  itk::SmartPointer<mitk::DataStorage> GetDataStorage(bool throwNull = true);

  /**
   * \brief Returns the combined modality set for the navigation step.
   *
   * \param throwNull if this method should throw an exception when the combined modality is null (default: true)
   * \return the combined modality set by SetCombinedModality(); can only be null if the parameter throwNull is set to false
   * \throws mitk::Exception if the combined modality is null and the parameter throwNull is set to true
   */
  itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice> GetCombinedModality(bool throwNull = true);

  /**
   * \brief Returns node with the given name and the given source node (parent) from the data storage.
   *
   * \param name the name of the node which should be got from the data storage
   * \param sourceName name of the source node under which the node should be searched
   * \return node with the given name or null if no node with the given name and source node could be found
   * \throws mitk::Exception if the data storage (can be set by SetDataStorage()) is null
   */
  itk::SmartPointer<mitk::DataNode> GetNamedDerivedNode(const char* name, const char* sourceName);

  /**
   * \brief Returns node with the given name and the given source node (parent) from the data storage.
   * The node is created if no matching node was found.
   *
   * \param name the name of the node which should be got from the data storage
   * \param sourceName name of the source node under which the node should be searched
   * \return node with the given name
   * \throws mitk::Exception if the data storage (can be set by SetDataStorage()) is null
   */
  itk::SmartPointer<mitk::DataNode> GetNamedDerivedNodeAndCreate(const char* name, const char* sourceName);

private:
  itk::SmartPointer<mitk::DataStorage>        m_DataStorage;
  itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice> m_CombinedModality;

  NavigationStepState                         m_NavigationStepState;
};

#endif // QMITKUSABSTRACTNAVIGATIONSTEP_H
