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

#ifndef QmitkMultiLabelSegmentationView_h
#define QmitkMultiLabelSegmentationView_h

#include <QmitkAbstractView.h>

#include "mitkSegmentationInteractor.h"
#include <mitkILifecycleAwarePart.h>

#include "ui_QmitkMultiLabelSegmentationControls.h"

// berry
#include <berryIBerryPreferences.h>

class QmitkRenderWindow;

/**
 * \ingroup ToolManagerEtAl
 * \ingroup org_mitk_gui_qt_multilabelsegmentation_internal
 */
class QmitkMultiLabelSegmentationView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkMultiLabelSegmentationView();
  virtual ~QmitkMultiLabelSegmentationView();

  typedef std::map<mitk::DataNode *, unsigned long> NodeTagMapType;

  // GUI setup
  void CreateQtPartControl(QWidget *parent);

  // ILifecycleAwarePart interface
public:
  void Activated();
  void Deactivated();
  void Visible();
  void Hidden();

  virtual int GetSizeFlags(bool width);
  virtual int ComputePreferredSize(bool width,
                                   int /*availableParallel*/,
                                   int /*availablePerpendicular*/,
                                   int preferredResult);

protected slots:

  // reaction to the shortcut for toggling the visibility of the working node
  void OnVisibilityShortcutActivated();

  // reaction to the shortcut for iterating over all labels
  void OnLabelToggleShortcutActivated();

  // reaction to the selection of any 2D segmentation tool
  void OnManualTool2DSelected(int id);

  // reaction to button "New Label"
  void OnNewLabel();

  // reaction to button "Show Label Table"
  void OnShowLabelTable(bool value);

  // reaction to button "New Segmentation Session"
  void OnNewSegmentationSession();

  // reaction to signal "goToLabel" from labelset widget
  void OnGoToLabel(const mitk::Point3D &pos);

  void OnResetView();

  // reaction to the button "Add Layer"
  void OnAddLayer();

  // reaction to the button "Delete Layer"
  void OnDeleteLayer();

  // reaction to the button "Previous Layer"
  void OnPreviousLayer();

  // reaction to the button "Next Layer"
  void OnNextLayer();

  // reaction to the combobox change "Change Layer"
  void OnChangeLayer(int);

  // reaction to the button "Deactive Active Tool"
  void OnDeactivateActiveTool();

  // reaction to the button "Lock exterior"
  void OnLockExteriorToggled(bool);

  // reaction to the selection of a new patient (reference) image in the DataStorage combobox
  void OnReferenceSelectionChanged(const mitk::DataNode* node);

  // reaction to the selection of a new Segmentation (working) image in the DataStorage combobox
  void OnSegmentationSelectionChanged(const mitk::DataNode* node);

  // reaction to ...
  void OnInterpolationSelectionChanged(int);

protected:

  // reimplemented from QmitkAbstractView
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes) override;

  // reimplemented from QmitkAbstractView
  void OnPreferencesChanged(const berry::IBerryPreferences* prefs) override;

  // reimplemented from QmitkAbstractView
  void NodeAdded(const mitk::DataNode* node) override;

  // reimplemented from QmitkAbstractView
  void NodeRemoved(const mitk::DataNode* node) override;

  void OnEstablishLabelSetConnection();

  void OnLooseLabelSetConnection();

  void SetFocus();

  void UpdateControls();

  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart);

  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart);

  void ResetMouseCursor();

  void SetMouseCursor(const us::ModuleResource, int hotspotX, int hotspotY);

  void InitializeListeners();

  /// \brief Checks if two images have the same size and geometry
  bool CheckForSameGeometry(const mitk::Image *image1, const mitk::Image *image2) const;

  QString GetLastFileOpenPath();

  void SetLastFileOpenPath(const QString &path);

  /// \brief the Qt parent of our GUI (NOT of this object)
  QWidget *m_Parent;

  /// \brief Qt GUI file
  Ui::QmitkMultiLabelSegmentationControls m_Controls;

  mitk::IRenderWindowPart *m_IRenderWindowPart;

  mitk::ToolManager *m_ToolManager;

  mitk::DataNode::Pointer m_ReferenceNode;
  mitk::DataNode::Pointer m_WorkingNode;

  mitk::NodePredicateAnd::Pointer m_ReferencePredicate;
  mitk::NodePredicateAnd::Pointer m_SegmentationPredicate;

  bool m_AutoSelectionEnabled;
  bool m_MouseCursorSet;

  mitk::SegmentationInteractor::Pointer m_Interactor;

  /**
   * Reference to the service registration of the observer,
   * it is needed to unregister the observer on unload.
   */
  us::ServiceRegistration<mitk::InteractionEventObserver> m_ServiceRegistration;
};

#endif // QmitkMultiLabelSegmentationView_h
