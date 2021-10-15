/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMULTILABELSEGMENTATIONVIEW_H
#define QMITKMULTILABELSEGMENTATIONVIEW_H

#include "ui_QmitkMultiLabelSegmentationControls.h"

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <berryIBerryPreferences.h>

/**
* @brief The multilabel segmentation view provides a set of tool to use different segmentation
*        algorithms.
*        It provides two selection widgets to load an image node and a segmentation node
*        on which to perform the segmentation. Creating new segmentation nodes is also possible.
*        The available segmentation tools are grouped into "2D"- and "3D"-tools.
*
*        Most segmentation tools / algorithms need some kind of user interaction, where the
*        user is asked to draw something in the image display or set some seed points / start values.
*        The tools also often provide additional propeties so that a user can modify the
*        algorithm's behavior.
*
*        In contrast to the basic QmitkSegmentationView this class additionally provides options to
*        work with different layers (create new layers, switch between layers).
*        Moreover, a multilabel widget displays all the existing labels of a multilabel segmentation
*        for the currently active layer.
*        The multilabel widget allows to control the labels by creatin new one, removing existing ones,
*        showing / hiding single labels, merging labels, (re-)naming them etc.
*
*        Interpolation for multilabel segmentations is currently not implemented.
*/
class QmitkMultiLabelSegmentationView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkMultiLabelSegmentationView();
  ~QmitkMultiLabelSegmentationView() override;

private Q_SLOTS:

  // reaction to the selection of a new reference image in the selection widget
  void OnReferenceSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  // reaction to the selection of a new segmentation image in the selection widget
  void OnSegmentationSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  // reaction to the shortcut for toggling the visibility of the working node
  void OnVisibilityShortcutActivated();

  // reaction to the shortcut for iterating over all labels
  void OnLabelToggleShortcutActivated();

  // reaction to the selection of any 2D segmentation tool
  void OnManualTool2DSelected(int id);

  // reaction to button "New Label"
  void OnNewLabel();

  // reaction to button "Save Preset"
  void OnSavePreset();

  // reaction to button "Load Preset"
  void OnLoadPreset();

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

  // reaction to the button "Lock exterior"
  void OnLockExteriorToggled(bool);

  // reaction to ...
  void OnInterpolationSelectionChanged(int);

private:

  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override {}

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  void OnPreferencesChanged(const berry::IBerryPreferences* prefs) override;

  void NodeAdded(const mitk::DataNode *node) override;

  void NodeRemoved(const mitk::DataNode* node) override;

  // make sure all images / segmentations look according to the user preference settings
  void ApplyDisplayOptions();

  // decorates a DataNode according to the user preference settings
  void ApplyDisplayOptions(mitk::DataNode* node);

  void OnEstablishLabelSetConnection();

  void OnLooseLabelSetConnection();

  void ResetMouseCursor();

  void SetMouseCursor(const us::ModuleResource&, int hotspotX, int hotspotY);

  void UpdateGUI();

  void ValidateSelectionInput();

  void UpdateWarningLabel(QString text);

  QWidget *m_Parent;

  Ui::QmitkMultiLabelSegmentationControls m_Controls;

  mitk::IRenderWindowPart *m_RenderWindowPart;

  mitk::ToolManager *m_ToolManager;

  mitk::DataNode::Pointer m_ReferenceNode;
  mitk::DataNode::Pointer m_WorkingNode;

  typedef std::map<mitk::DataNode *, unsigned long> NodeTagMapType;
  NodeTagMapType m_WorkingDataObserverTags;
  unsigned int m_RenderingManagerObserverTag;

  mitk::NodePredicateAnd::Pointer m_ReferencePredicate;
  mitk::NodePredicateAnd::Pointer m_SegmentationPredicate;

  bool m_AutoSelectionEnabled;
  bool m_MouseCursorSet;

};

#endif // QMITKMULTILABELSEGMENTATIONVIEW_H
