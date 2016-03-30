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


#ifndef __Q_MITK_MATCHPOINT_FRAME_CORRECTION_H
#define __Q_MITK_MATCHPOINT_FRAME_CORRECTION_H

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkMatchPointFrameCorrectionControls.h"

#include <mitkImage.h>

// MatchPoint
#include <mapDeploymentDLLInfo.h>
#include <mapDeploymentDLLHandle.h>
#include <mapRegistrationAlgorithmBase.h>
#include <mapIterativeAlgorithmInterface.h>
#include <mapMultiResRegistrationAlgorithmInterface.h>
#include <mapStoppableAlgorithmInterface.h>

#include <mitkMAPRegistrationWrapper.h>
#include <QmitkFramesRegistrationJob.h>

/*!
\brief View for motion artefact correction of images.

The view utalizes MatchPoint registration algorithms and the mitk::TimeFramesRegistrationHelper and implemnts the GUI
business logic to make frame correction aka motion artefact correction on 3D+t images.

*/
class QmitkMatchPointFrameCorrection : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  /**
  * Creates smartpointer typedefs
  */
  berryObjectMacro(QmitkMatchPointFrameCorrection)

  QmitkMatchPointFrameCorrection();
  ~QmitkMatchPointFrameCorrection();

protected slots:

  /**
  * @brief Connect all GUI elements to its corresponding slots
  */
  virtual void CreateConnections();

  /// \brief Called when the user clicks the GUI button
  void OnMaskCheckBoxToggeled(bool checked);
  void OnLoadAlgorithmButtonPushed();
  void OnSelectedAlgorithmChanged();

  void OnStartRegBtnPushed();
  void OnSaveLogBtnPushed();

  void OnFramesSelectAllPushed();
  void OnFramesDeSelectAllPushed();
  void OnFramesInvertPushed();

  void OnRegJobError(QString err);
  void OnRegJobFinished();
  void OnMapJobError(QString err);
  void OnMapResultIsAvailable(mitk::Image::Pointer spMappedData,
                              const QmitkFramesRegistrationJob* job);
  void OnAlgorithmIterated(QString info, bool hasIterationCount, unsigned long currentIteration);
  void OnLevelChanged(QString info, bool hasLevelCount, unsigned long currentLevel);
  void OnAlgorithmStatusChanged(QString info);
  void OnAlgorithmInfo(QString info);
  void OnFrameProcessed(double progress);
  void OnFrameRegistered(double progress);
  void OnFrameMapped(double progress);

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

  /**configure the frame list widget based on the selected target.*/
  void ConfigureFrameList();

  /**generates the ignore list based on the frame list widget selection.*/
  mitk::TimeFramesRegistrationHelper::IgnoreListType GenerateIgnoreList() const;

  /** Methods returns a list of all nodes in the data manager containing a registration wrapper.
    * The list may be empty.*/
  mitk::DataStorage::SetOfObjects::Pointer GetRegNodes() const;

  /** Returns a proposal for a (unique) default reg job name */
  std::string GetDefaultJobName() const;

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

  friend struct berry::SelectionChangedAdapter<QmitkMatchPointFrameCorrection>;
  /** @brief this pointer holds the algorithm selection listener */
  QScopedPointer<berry::ISelectionListener> m_AlgorithmSelectionListener;

  ::map::deployment::DLLHandle::Pointer m_LoadedDLLHandle;
  ::map::algorithm::RegistrationAlgorithmBase::Pointer m_LoadedAlgorithm;
  ::map::deployment::DLLInfo::ConstPointer m_SelectedAlgorithmInfo;

  typedef map::algorithm::facet::IterativeAlgorithmInterface IIterativeAlgorithm;
  typedef map::algorithm::facet::MultiResRegistrationAlgorithmInterface IMultiResAlgorithm;
  typedef map::algorithm::facet::StoppableAlgorithmInterface IStoppableAlgorithm;

  mitk::DataNode::Pointer m_spSelectedTargetNode;
  /*Data of the selected target node that should be used for registration.
  Can be the direct return of node->GetData(), but can also be a sub
  set (like a special time frame).*/
  mitk::BaseData::ConstPointer m_spSelectedTargetData;

  mitk::DataNode::Pointer m_spSelectedTargetMaskNode;
  mitk::Image::ConstPointer m_spSelectedTargetMaskData;

  // boolean variables to control visibility of GUI elements
  bool m_CanLoadAlgorithm;
  bool m_ValidInputs;
  bool m_Working;

  QWidget* m_Parent;
  Ui::MatchPointFrameCorrectionControls m_Controls;
};

#endif // MatchPoint_h

