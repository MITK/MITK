/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkMatchPointRegistrationManipulator_h
#define QmitkMatchPointRegistrationManipulator_h

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include <QmitkSliceNavigationListener.h>
#include <mitkMAPRegistrationWrapper.h>
#include <mitkRegistrationManipulationInteractor.h>
#include <itkEuler3DTransform.h>

#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

#include <QList>
#include <memory>

namespace Ui
{
  class MatchPointRegistrationManipulatorControls;
}

class QmitkMappingJob;

/*!
\brief QmitkMatchPointRegistrationManipulator

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

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

  void OnInteractionToolToggled(bool checked);
  void OnPreview3DToggled(bool checked);
  void OnTranslateMovingCenterToNavigator();

  void OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob* job);

  void Error(QString msg);

protected:

  void NodeRemoved(const mitk::DataNode* node) override;

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  std::unique_ptr<Ui::MatchPointRegistrationManipulatorControls> m_Controls;

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

  /** Activates the mouse interaction tool: saves display interaction configs,
   *  creates the interactor, connects events. */
  void ActivateInteractionTool();

  /** Deactivates the mouse interaction tool: restores display interaction configs,
   *  disconnects events, removes interactor. */
  void DeactivateInteractionTool();

  /** Updates the center-of-rotation crosshair indicator node. */
  void UpdateCenterOfRotationIndicator();

  /** Called by ITK observer when the interactor emits a translation delta. */
  void OnInteractorTranslation();

  /** Called by ITK observer when the interactor emits a rotation delta. */
  void OnInteractorRotation();

  /** Called by ITK observer when the interactor emits a position-select event. */
  void OnInteractorSelectPosition();

  /** Creates the 3D preview clone and wireframe nodes. Only acts if session is active and not already previewing. */
  void Start3DPreview();

  /** Removes 3D preview nodes from DataStorage and restores visibility. */
  void Stop3DPreview();

  /** Resets preview geometry from stored original and composes current registration transform. */
  void Update3DPreviewGeometry();

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

  // Mouse interaction tool members
  mitk::RegistrationManipulationInteractor::Pointer m_Interactor;
  mitk::DataNode::Pointer m_CenterOfRotationIndicatorNode;
  bool m_InteractionToolActive = false;

  // 3D preview members
  mitk::DataNode::Pointer m_3DPreviewCloneNode;
  mitk::DataNode::Pointer m_3DPreviewWireframeNode;
  vtkSmartPointer<vtkMatrix4x4> m_OriginalMovingVtkMatrix;
  bool m_3DPreviewActive = false;

  // ITK observer tags for interactor event connections
  unsigned long m_TranslationObserverTag = 0;
  unsigned long m_RotationObserverTag = 0;
  unsigned long m_SelectPositionObserverTag = 0;

};

#endif
