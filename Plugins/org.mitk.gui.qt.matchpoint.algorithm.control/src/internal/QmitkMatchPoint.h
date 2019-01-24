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

#ifndef __Q_MITK_MATCHPOINT_H
#define __Q_MITK_MATCHPOINT_H

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkMatchPointControls.h"

#include <mitkImage.h>

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
\brief MatchPoint

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkMatchPoint : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  /**
  * Creates smartpointer typedefs
  */
  berryObjectMacro(QmitkMatchPoint)

  QmitkMatchPoint();
  ~QmitkMatchPoint();

protected slots:

  /**
  * @brief Connect all GUI elements to its corresponding slots
  */
  virtual void CreateConnections();

  /// \brief Called when the user clicks the GUI button
  void OnLoadAlgorithmButtonPushed();
  void OnSelectedAlgorithmChanged();
  void OnNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  void OnStartRegBtnPushed();
  void OnStopRegBtnPushed();
  void OnSaveLogBtnPushed();

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

  /**
  * @brief Adapt the visibility of GUI elements depending on the current data  loaded
  */
  void AdaptFolderGUIElements();

  void Error(QString msg);

  void UpdateAlgorithmList();

  /**
  * checks if appropriated nodes are selected in the data manager. If nodes are selected,
  * they are stored m_MovingData and m_TargetData. It also sets the info lables accordingly.
  * @return True: All inputs are set and valid (images). False: At least one input is not set
  * or invalid */
  bool CheckInputs();

  /**
  * Updates the state of registration control buttons. Regarding to selected
  * inputs, loaded algorithm and its state.*/
  void ConfigureRegistrationControls();

  /**
  * Configures the progress bars according to the chosen algorithm.
  */
  void ConfigureProgressInfos();

  /**
  Configure the node selectors predicates and informations according to the selected algorithm.
  */
  void ConfigureNodeSelectors();

  /** Methods returns a list of all nodes in the data manager containing a registration wrapper.
    * The list may be empty.*/
  mitk::DataStorage::SetOfObjects::Pointer GetRegNodes() const;

  /** Returns a proposal for a (unique) default reg job name */
  std::string GetDefaultRegJobName() const;

  /** Returns the display name of the passed node. Normally it is just node->GetName().
   * if the node contains a point set it is additionally checked if the point set node
   * has a source node and its name will be added in parentheses.*/
  std::string GetInputNodeDisplayName(const mitk::DataNode* node) const;

  /** Returns the Pointer to the DLL info of the algorithm currently selected by the system.
  The info is received via m_AlgorithmSelectionListener.
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

  friend struct berry::SelectionChangedAdapter<QmitkMatchPoint>;

  QWidget* m_Parent;

  /** @brief this pointer holds the algorithm selection listener */
  QScopedPointer<berry::ISelectionListener> m_AlgorithmSelectionListener;

  ::map::deployment::DLLHandle::Pointer m_LoadedDLLHandle;
  ::map::algorithm::RegistrationAlgorithmBase::Pointer m_LoadedAlgorithm;
  ::map::deployment::DLLInfo::ConstPointer m_SelectedAlgorithmInfo;

  typedef map::algorithm::facet::IterativeAlgorithmInterface IIterativeAlgorithm;
  typedef map::algorithm::facet::MultiResRegistrationAlgorithmInterface IMultiResAlgorithm;
  typedef map::algorithm::facet::StoppableAlgorithmInterface IStoppableAlgorithm;

  mitk::DataNode::Pointer m_spSelectedTargetNode;
  mitk::DataNode::Pointer m_spSelectedMovingNode;
  /*Data of the selected target node that should be used for registration.
  Can be the direct return of node->GetData(), but can also be a sub
  set (like a special time frame).*/
  mitk::BaseData::ConstPointer m_spSelectedTargetData;
  /*Data of the selected moving node that should be used for registration.
  Can be the direct return of node->GetData(), but can also be a sub
  set (like a special time frame).*/
  mitk::BaseData::ConstPointer m_spSelectedMovingData;

  mitk::DataNode::Pointer m_spSelectedTargetMaskNode;
  mitk::DataNode::Pointer m_spSelectedMovingMaskNode;
  /*Data of the selected target mask node that should be used for registration.
  Can be the direct return of node->GetData(), but can also be a sub
  set (like a special time frame).*/
  mitk::Image::ConstPointer m_spSelectedTargetMaskData;
  /*Data of the selected moving mask node that should be used for registration.
  Can be the direct return of node->GetData(), but can also be a sub
  set (like a special time frame).*/
  mitk::Image::ConstPointer m_spSelectedMovingMaskData;

  // boolean variables to control visibility of GUI elements
  bool m_CanLoadAlgorithm;
  bool m_ValidInputs;
  bool m_Working;

  Ui::MatchPointAdvancedControls m_Controls;
};

#endif // MatchPoint_h

