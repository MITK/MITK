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

/**
 * \brief Workbench view for interactive manual refinement of a MatchPoint registration.
 *
 * The view drives a manipulation session in which the user can adjust a 3D rigid
 * transform between a moving and a target image. It exposes:
 *   - Slider/spin-box manipulation via QmitkRegistrationManipulationWidget.
 *   - An optional mouse-driven interaction tool (modifier+drag) backed by
 *     mitk::RegistrationManipulationInteractor.
 *   - An optional 3D preview (transformed clone of the moving image plus a
 *     wireframe bounding box) that updates live with the current transform.
 *   - A "translate moving image center to navigator position" shortcut.
 *   - A configurable center of rotation (moving image center, world origin, or
 *     navigator position) with a persistent on-screen indicator while the
 *     session is active.
 *
 * The session can either start from an identity transform or be initialized
 * with a previously selected registration as baseline. Storing the result
 * creates a new registration node in the data storage and optionally maps the
 * moving image and/or refines its geometry.
 *
 * \ingroup ${plugin_target}_internal
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

  /** Starts a new manipulation session based on the currently selected nodes. */
  void OnStartBtnPushed();
  /** Cancels the current manipulation session and discards all changes. */
  void OnCancelBtnPushed();
  /** Stores the current manipulation result as a new registration node and optionally
   *  triggers image mapping and/or geometry refinement on the moving image. */
  void OnStoreBtnPushed();
  /** Reacts to changes in the evaluation settings widget by requesting a render update. */
  void OnSettingsChanged(mitk::DataNode*);

  /** Reacts to switching between "new registration" and "selected pre-registration"
   *  as the baseline source. */
  void OnRegSourceChanged();
  /** Reacts to changes in any of the node selectors (registration, moving, target). */
  void OnNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  /** Reacts to transform changes from the manipulation widget; updates the
   *  evaluation node, the optional 3D preview, and the center indicator. */
  void OnRegistrationChanged();

  /** Reacts to a change of the center-of-rotation mode combo box. */
  void OnCenterTypeChanged(int);

  /** Reacts to a slice/time change in the active render window part; relevant for
   *  the "navigator position" center mode and for the evaluation node position. */
  void OnSliceChanged();

  /** Toggles the mouse interaction tool: activates it if \p checked and a session
   *  is active, otherwise deactivates it. */
  void OnInteractionToolToggled(bool checked);
  /** Toggles the 3D preview (transformed clone plus wireframe) of the moving image. */
  void OnPreview3DToggled(bool checked);
  /** Applies an incremental translation that shifts the currently transformed
   *  moving-image center onto the current navigator position. */
  void OnTranslateMovingCenterToNavigator();

  /** Adds a mapped result image to the data storage when an asynchronous mapping
   *  job triggered from OnStoreBtnPushed() finishes successfully. */
  void OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob* job);

  /** Reports an error message to the status bar and the MITK log. */
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

  /** Activates the mouse interaction tool: creates the interactor, suppresses the
   *  conflicting default display interactions, connects observers, and pushes the
   *  manipulation cursor. */
  void ActivateInteractionTool();

  /** Deactivates the mouse interaction tool: pops the manipulation cursor, removes
   *  observers, restores the default display interactions, and releases the
   *  interactor. */
  void DeactivateInteractionTool();

  /** Creates or updates the on-screen center-of-rotation indicator (a yellow
   *  diamond-shaped point glyph). The indicator position depends on the active
   *  center-of-rotation mode and, in "moving image center" mode, follows the
   *  current registration transform. */
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
