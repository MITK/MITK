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



#ifndef __Q_MITK_MATCHPOINT_BATCH_PROCESSOR_H
#define __Q_MITK_MATCHPOINT_BATCH_PROCESSOR_H

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkMatchPointBatchProcessor.h"

// MatchPoint
#include <mapDeploymentDLLInfo.h>
#include <mapDeploymentDLLHandle.h>
#include <mapRegistrationAlgorithmBase.h>
#include <mapIterativeAlgorithmInterface.h>
#include <mapMultiResRegistrationAlgorithmInterface.h>
#include <mapStoppableAlgorithmInterface.h>

#include <mitkMAPRegistrationWrapper.h>

class QmitkRegistrationJob;
class QmitkMappingJob;

/*!
\brief View for registration batch processing based on MatchPoint

This view composes the GUI and implements the business logic needed to allow registration
batch processing (one target, many moving images).

\ingroup ${plugin_target}_internal
*/
class QmitkMatchPointBatchProcessor : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  /**
  * Creates smartpointer typedefs
  */
  berryObjectMacro(QmitkMatchPointBatchProcessor)

  QmitkMatchPointBatchProcessor();
  ~QmitkMatchPointBatchProcessor();

protected slots:

  /**
    * @brief Connect all GUI elements to its corresponding slots
    */
  virtual void CreateConnections();

  /// \brief Called when the user clicks the GUI button

  void OnLoadAlgorithmButtonPushed();
  void OnSelectedAlgorithmChanged();

  void OnLockMovingButtonPushed();
  void OnLockTargetButtonPushed();

  void OnStartRegBtnPushed();
  void OnStopRegBtnPushed();

  void OnRegJobError(QString err);
  void OnRegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer spResultRegistration,
                              const QmitkRegistrationJob* pRegJob);
  void OnRegJobFinished();
  void OnMapJobError(QString err);
  void OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob* job);
  void OnAlgorithmIterated(QString info, bool hasIterationCount, unsigned long currentIteration);
  void OnLevelChanged(QString info, bool hasLevelCount, unsigned long currentLevel);
  void OnAlgorithmStatusChanged(QString info);
  void OnAlgorithmInfo(QString info);

protected:
  virtual void CreateQtPartControl(QWidget* parent);

  virtual void SetFocus();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer>& nodes);

private:
  typedef QList<mitk::DataNode::Pointer> NodeListType;

  /** Spawns a registration job for the next selected moving node (see m_nextNodeToSpawn).
   * If no nodes are left, no job will be spawned and false is returned.
   * If at least one node is left, a job will be spawned, m_nextNodeToSpawn will be incremented
   * and true will be returned.*/
  bool SpawnNextJob();

  /**
  * @brief Adapt the visibility of GUI elements depending on the current data  loaded
  */
  void UpdateAlgorithmSelectionGUI();


  void Error(QString msg);

  void UpdateAlgorithmList();

  /**
  * checks if appropriated nodes are selected in the data manager. If nodes are selected,
  * they are stored in m_spSelectedTargetNode and m_selectedMovingNodes. It also sets the info labels accordingly.*/
  void CheckInputs();

  /**
  * Updates the state of registration control buttons. Regarding to selected
  * inputs, loaded algorithm and its state.*/
  void ConfigureRegistrationControls();

  /**
  * Configures the progress bars according to the chosen algorithm.
  */
  void ConfigureProgressInfos();

  /** Methods returns a list of all nodes selected in the data manager that do
    * NOT contain registration wrappers. The list may be empty if nothing is
    * selected or no appropriate data node is selected.*/
  NodeListType GetSelectedDataNodes();

  /** Methods returns a list of all nodes in the data manager containing a registration wrapper.
    * The list may be empty.*/
  mitk::DataStorage::SetOfObjects::Pointer GetRegNodes() const;

  /** Returns a proposal for a (unique) default reg job name */
  std::string GetDefaultRegJobName() const;

  /** Returns the Pointer to the DLL info of the algorithm currently selected by the system.
  The info is received via m_SelectionListener.
  @return If there is no item selected the returning pointer
  will be null.
  */
  const map::deployment::DLLInfo* GetSelectedAlgorithmDLL() const;

  //! [Qt Selection Listener method and pointer]
  /**
  * @brief Method of berry::ISelectionListener that implements the selection listener functionality.
  * @param sourcepart The workbench part responsible for the selection change.
  * @param selection This parameter holds the current selection.
  *
  * @see ISelectionListener
  */
  void OnAlgorithmSelectionChanged(const berry::IWorkbenchPart::Pointer& sourcepart,
                                   const berry::ISelection::ConstPointer& selection);

  void UpdateAlgorithmSelection(berry::ISelection::ConstPointer selection);

  friend struct berry::SelectionChangedAdapter<QmitkMatchPointBatchProcessor>;
  /** @brief this pointer holds the algorithm selection listener */
  QScopedPointer<berry::ISelectionListener> m_AlgorithmSelectionListener;

  ::map::deployment::DLLHandle::Pointer m_LoadedDLLHandle;
  ::map::algorithm::RegistrationAlgorithmBase::Pointer m_LoadedAlgorithm;
  ::map::deployment::DLLInfo::ConstPointer m_SelectedAlgorithmInfo;

  typedef map::algorithm::facet::IterativeAlgorithmInterface IIterativeAlgorithm;
  typedef map::algorithm::facet::MultiResRegistrationAlgorithmInterface IMultiResAlgorithm;
  typedef map::algorithm::facet::StoppableAlgorithmInterface IStoppableAlgorithm;

  /**All selected nodes stored by OnSelectionChanged()*/
  NodeListType m_currentlySelectedNodes;

  mitk::DataNode::Pointer m_spSelectedTargetNode;
  NodeListType m_selectedMovingNodes;
  /**indicates the next selected moving nodes that should be spawned as job.*/
  unsigned int m_nextNodeToSpawn;

  // boolean variables to control visibility of GUI elements
  bool m_CanLoadAlgorithm;
  bool m_ValidInputs;
  bool m_Working;

  QWidget* m_Parent;
  Ui::MatchPointAdvancedControls m_Controls;
};

#endif // MatchPoint_h

