/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSEGMENTATIONVIEW_H
#define QMITKSEGMENTATIONVIEW_H

#include "ui_QmitkSegmentationViewControls.h"

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <berryIBerryPreferences.h>

/**
* @brief The segmentation view provides a set of tool to use different segmentation algorithms.
*        It provides two selection widgets to load an image node and a segmentation node
*        on which to perform the segmentation. Creating new segmentation nodes is also possible.
*        The available segmentation tools are grouped into "2D"- and "3D"-tools.
*
*        Most segmentation tools / algorithms need some kind of user interaction, where the
*        user is asked to draw something in the image display or set some seed points / start values.
*        The tools also often provide additional propeties so that a user can modify the
*        algorithm's behavior.
*
*        This class additionally provides options to work with different layers (create new layers,
*        switch between layers).
*        Moreover, a multilabel widget displays all the existing labels of a multilabel segmentation
*        for the currently active layer.
*        The multilabel widget allows to control the labels by creatin new one, removing existing ones,
*        showing / hiding single labels, merging labels, (re-)naming them etc.
*
*        Additionally the view provides an option to create "2D"- and "3D"-interpolations between
*        neighboring segmentation masks on unsegmented slices.
*        Interpolation for multilabel segmentations is currently not implemented.
*/
class QmitkSegmentationView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkSegmentationView();
  ~QmitkSegmentationView() override;

private Q_SLOTS:

  // reaction to the selection of a new reference image in the selection widget
  void OnReferenceSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  // reaction to the selection of a new segmentation image in the selection widget
  void OnSegmentationSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  // reaction to the shortcut ("CTRL+H") for toggling the visibility of the working node
  void OnVisibilityShortcutActivated();

  // reaction to the shortcut ("CTRL+L") for iterating over all labels
  void OnLabelToggleShortcutActivated();

  // reaction to the button "New segmentation"
  void OnNewSegmentation();

  void OnManualTool2DSelected(int id);

  void OnShowMarkerNodes(bool);

  void OnLayersChanged();

  void OnLabelsChanged();

  void OnShowLabelTable(bool);

  void OnGoToLabel(const mitk::Point3D &pos);

  void OnLabelSetWidgetReset();

private:

  void CreateQtPartControl(QWidget* parent) override;

  void SetFocus() override {}

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  void OnPreferencesChanged(const berry::IBerryPreferences* prefs) override;

  void NodeAdded(const mitk::DataNode* node) override;

  void NodeRemoved(const mitk::DataNode* node) override;

  void OnEstablishLabelSetConnection();

  void OnLooseLabelSetConnection();

  // make sure all images / segmentations look according to the user preference settings
  void ApplyDisplayOptions();

  // decorates a DataNode according to the user preference settings
  void ApplyDisplayOptions(mitk::DataNode* node);

  // If a contourmarker is selected, the plane in the related widget will be reoriented according to the marker`s geometry
  void OnContourMarkerSelected(const mitk::DataNode* node);

  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes) override;

  void ResetMouseCursor();

  void SetMouseCursor(const us::ModuleResource&, int hotspotX, int hotspotY);

  void UpdateGUI();

  void ValidateSelectionInput();

  void UpdateWarningLabel(QString text);

  std::string GetDefaultLabelSetPreset() const;

  QWidget* m_Parent;

  Ui::QmitkSegmentationViewControls* m_Controls;

  mitk::IRenderWindowPart* m_RenderWindowPart;

  mitk::ToolManager* m_ToolManager;

  mitk::DataNode::Pointer m_ReferenceNode;
  mitk::DataNode::Pointer m_WorkingNode;

  typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;
  NodeTagMapType m_WorkingDataObserverTags;
  unsigned int m_RenderingManagerObserverTag;

  mitk::NodePredicateAnd::Pointer m_ReferencePredicate;
  mitk::NodePredicateAnd::Pointer m_SegmentationPredicate;

  bool m_DrawOutline;
  bool m_SelectionMode;
  bool m_MouseCursorSet;

  QString m_LabelSetPresetPreference;
};

#endif // QMITKSEGMENTATIONVIEW_H
