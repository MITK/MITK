/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __Q_MITK_MATCHPOINT_REGISTRATION_MANIPULATOR_H
#define __Q_MITK_MATCHPOINT_REGISTRATION_MANIPULATOR_H

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include <QmitkSliceNavigationListener.h>
#include <mitkMAPRegistrationWrapper.h>
#include <itkEuler3DTransform.h>
#include "ui_QmitkMatchPointRegistrationManipulator.h"


class QmitkMappingJob;

/*!
\brief QmitkMatchPointRegistrationManipulator

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkMatchPointRegistrationManipulator : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  /**
  * Creates smartpointer typedefs
  */
  berryObjectMacro(QmitkMatchPointRegistrationManipulator);

  QmitkMatchPointRegistrationManipulator();
  ~QmitkMatchPointRegistrationManipulator() override;

  void CreateQtPartControl(QWidget *parent) override;

  protected slots:

    /// \brief Called when the user clicks the GUI button

  void OnStartBtnPushed();
  void OnCancelBtnPushed();
  void OnStoreBtnPushed();
  void OnSettingsChanged(mitk::DataNode*);

  void OnRegSourceChanged();
  void OnNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  void OnRegistrationChanged();

  void OnCenterTypeChanged(int);

  void OnSliceChanged();

  void OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob* job);

  void Error(QString msg);

protected:

  void NodeRemoved(const mitk::DataNode* node) override;

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  Ui::MatchPointRegistrationManipulatorControls m_Controls;

private:
  QWidget *m_Parent;



  /** Methods returns a list of all eval nodes in the data manager.*/
  QList<mitk::DataNode::Pointer> GetEvalNodes();

  /**
  * Checks if appropriated nodes are selected in the data manager. If nodes are selected,
  * they are stored m_spSelectedRegNode, m_spSelectedInputNode and m_spSelectedRefNode.
  * They are also checked for validity.*/
  void CheckInputs();

  /**
  * Updates the state of controls regarding to the state of the view and it objects.*/
  void ConfigureControls();

  /**
  Configure the node selectors predicates according to the selected algorithm.
  */
  void ConfigureNodePredicates();

  /** Initialize the state of the view, so the manipulation can start.*/
  void InitSession();

  /** Stops session, removes all obsolete members (e.g. RegEvalObject). After that the view is in a valid but inactive state.*/
  void StopSession();

  void ConfigureTransformCenter(int centerType);

  mitk::DataNode::Pointer m_EvalNode;

  QmitkSliceNavigationListener m_SliceChangeListener;

  itk::TimeStamp m_selectedNodeTime;
  itk::TimeStamp m_currentPositionTime;

  bool m_activeManipulation;

  /** @brief currently valid selected position in the inspector*/
  mitk::Point3D m_currentSelectedPosition;
  /** @brief currently selected timepoint*/
  mitk::TimePointType m_currentSelectedTimePoint;

  mitk::DataNode::Pointer m_SelectedPreRegNode;

  mitk::DataNode::Pointer m_SelectedMovingNode;
  mitk::DataNode::Pointer m_SelectedTargetNode;


  mitk::MAPRegistrationWrapper::Pointer m_CurrentRegistrationWrapper;
  map::core::RegistrationBase::Pointer m_CurrentRegistration;
  using MAPRegistrationType = map::core::Registration<3, 3>;
  MAPRegistrationType::Pointer m_SelectedPreReg;

  bool m_internalUpdate;
  static const std::string HelperNodeName;
};

#endif // MatchPoint_h

