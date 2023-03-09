/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkMatchPointRegistrationEvaluator_h
#define QmitkMatchPointRegistrationEvaluator_h

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include <QmitkSliceNavigationListener.h>

#include "ui_QmitkMatchPointRegistrationEvaluator.h"

/*!
\brief QmitkMatchPointRegistrationEvaluator

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\ingroup ${plugin_target}_internal
*/
class QmitkMatchPointRegistrationEvaluator : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  /**
  * Creates smartpointer typedefs
  */
  berryObjectMacro(QmitkMatchPointRegistrationEvaluator);

  QmitkMatchPointRegistrationEvaluator();
  ~QmitkMatchPointRegistrationEvaluator() override;

  void CreateQtPartControl(QWidget *parent) override;

protected slots:

    /// \brief Called when the user clicks the GUI button

  void OnEvalBtnPushed();
  void OnStopBtnPushed();
  void OnSettingsChanged(mitk::DataNode*);

  void OnSliceChanged();

  void OnNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

protected:
  void NodeRemoved(const mitk::DataNode* node) override;

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  Ui::MatchPointRegistrationEvaluatorControls m_Controls;

private:
  QWidget *m_Parent;

  void Error(QString msg);

  /**
  * Checks if appropriated nodes are selected in the data manager. If nodes are selected,
  * they are stored m_spSelectedRegNode, m_spSelectedInputNode and m_spSelectedRefNode.
  * They are also checked for vadility and stored in m_ValidInput,... .
  * It also sets the info lables accordingly.*/
  void CheckInputs();

  /**
  * Updates the state of controls regarding to selected eval object.*/
  void ConfigureControls();

  /**
  Configure the node selectors predicates according to the selected algorithm.
  */
  void ConfigureNodePredicates();

  mitk::DataNode::Pointer m_selectedEvalNode;

  QmitkSliceNavigationListener m_SliceChangeListener;

  itk::TimeStamp m_selectedNodeTime;
  itk::TimeStamp m_currentPositionTime;

  bool m_activeEvaluation;

  /** @brief currently valid selected position in the inspector*/
  mitk::Point3D m_currentSelectedPosition;
  /** @brief currently selected timepoint*/
  mitk::TimePointType m_currentSelectedTimePoint;

  mitk::DataNode::Pointer m_spSelectedRegNode;
  mitk::DataNode::Pointer m_spSelectedMovingNode;
  mitk::DataNode::Pointer m_spSelectedTargetNode;

  static const std::string HelperNodeName;
};

#endif
